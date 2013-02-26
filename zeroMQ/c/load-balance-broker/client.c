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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <zmq.h>
#include <utils.h>

#include "lbbroker.h"


static void* 
client_routine(void* arg)
{
  void* ctx;
  void* client;
  char identity[16];
  char buf[128];

  ctx = zmq_ctx_new();
  client = zmq_socket(ctx, ZMQ_REQ);
  sprintf(identity, "%p", client);
  set_identity(client, identity);
  zmq_connect(client, "ipc://front.ipc");

  sprintf(buf, "HELLO");
  zmq_send(client, buf, strlen(buf), 0);
  memset(buf, 0, sizeof(buf));
  zmq_recv(client, buf, sizeof(buf), 0);
  fprintf(stdout, "client[%p]: %s\n", client, buf);

  zmq_close(client);
  zmq_ctx_destroy(ctx);

  return NULL;
}


int 
start_client(int num_client)
{
  int i;

  for (i = 0; i < num_client; ++i) {
    pthread_t tid;
    pthread_create(&tid, NULL, client_routine, NULL);
  }
  return 0;
}
