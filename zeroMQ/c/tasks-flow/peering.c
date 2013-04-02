/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <czmq.h>



#define NUM_CLIENTS   (10)
#define NUM_WORKERS   (3)
#define WORKER_READY  "\001"


static char* self;

static void* 
client_routine(void* arg)
{
  char* reply;
  zctx_t* ctx = zctx_new();
  void* client = zsocket_new(ctx, ZMQ_REQ);
  zsocket_connect(client, "ipc://%s-localfe.ipc", self);

  while (1) {
    zstr_send(client, "HELLO");
    reply = zstr_recv(client);

    if (!reply)
      break;
    fprintf(stdout, "client: %s\n", reply);
    free(reply);
    zclock_sleep(1000);
  }

  zctx_destroy(&ctx);
  return NULL;
}

static void* 
worker_routine(void* arg)
{
  zmsg_t* msg;
  zframe_t* frame;
  zctx_t* ctx = zctx_new();
  void* worker = zsocket_new(ctx, ZMQ_REQ);
  zsocket_connect(worker, "ipc://%s-localbe.ipc", self);

  frame = zframe_new(WORKER_READY, 1);
  zframe_send(&frame, worker, 0);

  while (1) {
    msg = zmsg_recv(worker);
    if (!msg)
      break;

    zframe_print(zmsg_last(msg), "Worker: ");
    zframe_reset(zmsg_last(msg), "OK", 2);
    zmsg_send(&msg, worker);
  }

  zctx_destroy(&ctx);
  return NULL;
}



int 
main(int argc, char* argv[])
{
  zctx_t* ctx;
  void* cloudfe;
  void* cloudbe;
  int i;
  void* localfe;
  void* localbe;
  int capacity = 0;
  zlist_t* workers;

  if (argc < 2) {
    fprintf(stderr, "syntax: peering me {you} ...\n");
    return 1;
  }
  self = argv[1];
  fprintf(stdout, "I: preparing broker at %s ...\n", self);
  srand((unsigned int)time(NULL));

  ctx = zctx_new();

  cloudfe = zsocket_new(ctx, ZMQ_ROUTER);
  zsockopt_set_identity(cloudfe, self);
  zsocket_bind(cloudfe, "ipc://%s-cloud.ipc", self);

  cloudbe = zsocket_new(ctx, ZMQ_ROUTER);
  zsockopt_set_identity(cloudbe, self);
  for (i = 2; i < argc; ++i) {
    char* peer = argv[i];
    fprintf(stdout, "I: connecting to cloud frontend at '%s'\n", peer);
    zsocket_connect(cloudbe, "ipc://%s-cloud.ipc", peer);
  }

  localfe = zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(localfe, "ipc://%s-localfe.ipc", self);
  localbe = zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(localbe, "ipc://%s-localbe.ipc", self);

  fprintf(stdout, "Press Enter when all brokers are started: ");
  getchar();

  for (i = 0; i < NUM_WORKERS; ++i)
    zthread_new(worker_routine, NULL);
  for (i = 0; i < NUM_CLIENTS; ++i)
    zthread_new(client_routine, NULL);

  workers = zlist_new();
  while (1) {
    zmsg_t* msg;
    zmq_pollitem_t backends[] = {
      {localbe, 0, ZMQ_POLLIN, 0}, 
      {cloudbe, 0, ZMQ_POLLIN, 0}, 
    };
    int r = zmq_poll(backends, 2, capacity ? 1000 * ZMQ_POLL_MSEC : -1);
    if (-1 == r)
      break;

    msg = NULL;
    if (backends[0].revents & ZMQ_POLLIN) {
      zframe_t* identity;
      zframe_t* frame;
      msg = zmsg_recv(localbe);
      if (!msg)
        break;

      identity = zmsg_unwrap(msg);
      zlist_append(workers, identity);
      ++capacity;

      frame = zmsg_first(msg);
      if (0 == memcmp(zframe_data(frame), WORKER_READY, 1))
        zmsg_destroy(&msg);
    }
    else if (backends[1].revents & ZMQ_POLLIN) {
      zframe_t* identity;
      msg = zmsg_recv(cloudbe);
      if (!msg)
        break;

      identity = zmsg_unwrap(msg);
      zframe_destroy(&identity);
    }
    for (i = 2; msg && i < argc; ++i) {
      char* data = (char*)zframe_data(zmsg_first(msg));
      size_t size = zframe_size(zmsg_first(msg));
      if (size == strlen(argv[i]) && 0 == memcmp(data, argv[i], size))
        zmsg_send(&msg, cloudfe);
    }
    if (msg) 
      zmsg_send(&msg, localfe);


    while (capacity) {
      int reroutable = 0;
      zmq_pollitem_t frontends[] = {
        {localfe, 0, ZMQ_POLLIN, 0}, 
        {cloudfe, 0, ZMQ_POLLIN, 0},
      };
      r = zmq_poll(frontends, 2, 0);
      assert(r >= 0);

      if (frontends[1].revents & ZMQ_POLLIN) {
        msg = zmsg_recv(cloudfe);
        reroutable = 0;
      }
      else if (frontends[0].revents & ZMQ_POLLIN) {
        msg = zmsg_recv(localfe);
        reroutable = 1;
      }
      else 
        break;


      if (0 != reroutable && argc > 2 && 0 == rand() % 5) {
        int random_peer = rand() % (argc - 2) + 2;
        zmsg_pushmem(msg, argv[random_peer], strlen(argv[random_peer]));
        zmsg_send(&msg, cloudbe);
      }
      else {
        zframe_t* frame = (zframe_t*)zlist_pop(workers);
        zmsg_wrap(msg, frame);
        zmsg_send(&msg, localbe);
        --capacity;
      }
    }
  }

  while (zlist_size(workers)) {
    zframe_t* frame = (zframe_t*)zlist_pop(workers);
    zframe_destroy(&frame);
  }
  zlist_destroy(&workers);

  zctx_destroy(&ctx);
  return 0;
}
