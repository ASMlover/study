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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <czmq.h>


#define WORKER_READY    ("\001")



static void* 
client_routine(void* arg)
{
  zctx_t* ctx = zctx_new();

  void* client = zsocket_new(ctx, ZMQ_REQ);
  zsocket_connect(client, "ipc://frontend.ipc");

  while (1) {
    char* reply;
    zstr_send(client, "Hello");
    reply = zstr_recv(client);

    if (NULL == reply)
      break;

    fprintf(stdout, "client: %s\n", reply);
    free(reply);
    sleep(1);
  }

  zctx_destroy(&ctx);

  return NULL;
}

static void* 
worker_routine(void* arg)
{
  zframe_t* frame;
  zctx_t* ctx = zctx_new();
  void* worker = zsocket_new(ctx, ZMQ_REQ);
  zsocket_connect(worker, "ipc://backend.ipc");


  frame = zframe_new(WORKER_READY, 1);
  zframe_send(&frame, worker, 0);

  while (1) {
    zmsg_t* msg = zmsg_recv(worker);
    if (NULL == msg)
      break;

    zframe_reset(zmsg_last(msg), "OK", 2);
    zmsg_send(&msg, worker);
  }

  zctx_destroy(&ctx);
  return NULL;
}



int 
main(int argc, char* argv[])
{
  int i, client_num, worker_num;
  zctx_t* ctx;
  void* frontend;
  void* backend;
  zlist_t* workers;

  if (argc < 3) {
    fprintf(stderr, "arguments error ...\n");
    return 1;
  }
  client_num = atoi(argv[1]);
  worker_num = atoi(argv[2]);

  ctx = zctx_new();


  frontend = zsocket_new(ctx, ZMQ_ROUTER);
  backend = zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(frontend, "ipc://frontend.ipc");
  zsocket_bind(backend, "ipc://backend.ipc");

  for (i = 0; i < client_num; ++i)
    zthread_new(client_routine, NULL);
  for (i = 0; i < worker_num; ++i)
    zthread_new(worker_routine, NULL);

  workers = zlist_new();
  while (1) {
    zmq_pollitem_t items[] = {
      {backend, 0, ZMQ_POLLIN, 0}, 
      {frontend, 0, ZMQ_POLLIN, 0}, 
    };
    int rc = zmq_poll(items, zlist_size(workers) ? 2 : 1, -1);
    if (-1 == rc)
      break;

    if (items[0].revents & ZMQ_POLLIN) {
      zmsg_t* msg;
      zframe_t* identity;
      zframe_t* frame;

      msg = zmsg_recv(backend);
      if (NULL == msg)
        break;
      identity = zmsg_unwrap(msg);
      zlist_append(workers, identity);

      frame = zmsg_first(msg);
      if (0 == memcmp(zframe_data(frame), WORKER_READY, 1))
        zmsg_destroy(&msg);
      else 
        zmsg_send(&msg, frontend);
    }
    if (items[1].revents & ZMQ_POLLIN) {
      zmsg_t* msg = zmsg_recv(frontend);
      if (NULL != msg) {
        zmsg_wrap(msg, (zframe_t*)zlist_pop(workers));
        zmsg_send(&msg, backend);
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
