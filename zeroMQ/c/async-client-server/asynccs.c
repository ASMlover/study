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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <czmq.h>




static void* 
client_routine(void* args)
{
  zctx_t* ctx = zctx_new();
  void* client;
  char identity[10];
  int i, request = 0;

  client = zsocket_new(ctx, ZMQ_DEALER);
  sprintf(identity, "%p", ctx);
  zsockopt_set_identity(client, identity);
  zsocket_connect(client, "tcp://localhost:5555");

  while (1) {
    zmq_pollitem_t items[] = {{client, 0, ZMQ_POLLIN, 0}};

    for (i = 0; i < 100; ++i) {
      zmq_poll(items, 1, 10 * ZMQ_POLL_MSEC);
      if (items[0].revents & ZMQ_POLLIN) {
        zmsg_t* msg = zmsg_recv(client);
        zframe_print(zmsg_last(msg), identity);
        zmsg_destroy(&msg);
      }
    }
    zstr_sendf(client, "request #%d", ++request);
  }

  zctx_destroy(&ctx);
  return NULL;
}


static void 
server_worker(void* arg, zctx_t* ctx, void* pipe)
{
  int i, replies;
  void* worker = zsocket_new(ctx, ZMQ_DEALER);
  zsocket_connect(worker, "inproc://backend");

  while (1) {
    zmsg_t* msg = zmsg_recv(worker);
    zframe_t* identity = zmsg_pop(msg);
    zframe_t* content = zmsg_pop(msg);
    assert(NULL != content);
    zmsg_destroy(&msg);

    replies = rand() % 5;
    for (i = 0; i < replies; ++i) {
      zclock_sleep(rand() % 1000 + 1);
      zframe_send(&identity, worker, ZFRAME_REUSE + ZFRAME_MORE);
      zframe_send(&content, worker, ZFRAME_REUSE);
    }

    zframe_destroy(&identity);
    zframe_destroy(&content);
  }
}


static void* 
server_routine(void* args)
{
  zctx_t* ctx = zctx_new();
  void* frontend;
  void* backend;
  int i;

  frontend = zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(frontend, "tcp://*:5555");

  backend = zsocket_new(ctx, ZMQ_DEALER);
  zsocket_bind(backend, "inproc://backend");

  for (i = 0; i < 5; ++i)
    zthread_fork(ctx, server_worker, NULL);

  zmq_proxy(frontend, backend, NULL);

  zctx_destroy(&ctx);
  return NULL;
}


int 
main(int argc, char* argv[])
{
  zthread_new(client_routine, NULL);
  zthread_new(client_routine, NULL);
  zthread_new(client_routine, NULL);
  zthread_new(server_routine, NULL);

  zclock_sleep(5000);

  return 0;
}
