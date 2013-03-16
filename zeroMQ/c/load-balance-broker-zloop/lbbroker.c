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


#define WORKER_READY  ("\001")



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

    fprintf(stdout, "Client: %s\n", reply);
    free(reply);
    sleep(1);
  }

  zctx_destroy(&ctx);
  return NULL;
}

static void* 
worker_routine(void* arg)
{
  zctx_t* ctx = zctx_new();
  void* worker;
  zframe_t* frame;

  worker = zsocket_new(ctx, ZMQ_REQ);
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


typedef struct lbbroker_s {
  void* frontend;
  void* backend;
  zlist_t* workers;
} lbbroker_t;


static int 
handle_frontend(zloop_t* loop, zmq_pollitem_t* poller, void* arg)
{
  lbbroker_t* self = (lbbroker_t*)arg;
  zmsg_t* msg = zmsg_recv(self->frontend);

  if (NULL != msg) {
    zmsg_wrap(msg, (zframe_t*)zlist_pop(self->workers));
    zmsg_send(&msg, self->backend);

    if (0 == zlist_size(self->workers)) {
      zmq_pollitem_t poller = {self->frontend, 0, ZMQ_POLLIN};
      zloop_poller_end(loop, &poller);
    }
  }

  return 0;
}

static int 
handle_backend(zloop_t* loop, zmq_pollitem_t* poller, void* arg)
{
  lbbroker_t* self = (lbbroker_t*)arg;
  zmsg_t* msg = zmsg_recv(self->backend);

  if (NULL != msg) {
    zframe_t* frame;
    zframe_t* identity = zmsg_unwrap(msg);
    zlist_append(self->workers, identity);

    if (1 == zlist_size(self->workers)) {
      zmq_pollitem_t poller = {self->frontend, 0, ZMQ_POLLIN};
      zloop_poller(loop, &poller, handle_frontend, self);
    }

    frame = zmsg_first(msg);
    if (0 == memcmp(zframe_data(frame), WORKER_READY, 1))
      zmsg_destroy(&msg);
    else 
      zmsg_send(&msg, self->frontend);
  }

  return 0;
}



int 
main(int argc, char* argv[])
{
  int i, client_num, worker_num;
  zctx_t* ctx;
  lbbroker_t* self = (lbbroker_t*)zmalloc(sizeof(lbbroker_t));
  zloop_t* reactor;
  zmq_pollitem_t poller;

  if (argc < 3) {
    fprintf(stderr, "arguments error ...\n");
    return 1;
  }
  client_num = atoi(argv[1]);
  worker_num = atoi(argv[2]);

  ctx = zctx_new();

  self->frontend = zsocket_new(ctx, ZMQ_ROUTER);
  self->backend = zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(self->frontend, "ipc://frontend.ipc");
  zsocket_bind(self->backend, "ipc://backend.ipc");

  for (i = 0; i < client_num; ++i)
    zthread_new(client_routine, NULL);
  for (i = 0; i < worker_num; ++i)
    zthread_new(worker_routine, NULL);

  self->workers = zlist_new();
  reactor = zloop_new();
  poller.socket = self->backend;
  poller.fd = 0;
  poller.events = ZMQ_POLLIN;

  zloop_poller(reactor, &poller, handle_backend, self);
  zloop_start(reactor);
  zloop_destroy(&reactor);

  while (zlist_size(self->workers)) {
    zframe_t* frame = (zframe_t*)zlist_pop(self->workers);
    zframe_destroy(&frame);
  }

  zlist_destroy(&self->workers);
  free(self);
  zctx_destroy(&ctx);
  return 0;
}
