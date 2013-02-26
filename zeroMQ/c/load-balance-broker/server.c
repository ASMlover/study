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
server_routine(void* arg)
{
  char identity[16];
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* server = zmq_socket(ctx, ZMQ_REQ);

  sprintf(identity, "%p", server);
  set_identity(server, identity);
  zmq_connect(server, "ipc://back.ipc");
  zmq_send(server, "READY", 5, 0);

  fprintf(stdout, "server [%p] init success ...\n", server);
  while (1) {
    memset(identity, 0, sizeof(identity));
    zmq_recv(server, identity, sizeof(identity), 0);
    zmq_recv(server, buf, sizeof(buf), 0);

    memset(buf, 0, sizeof(buf));
    zmq_recv(server, buf, sizeof(buf), 0);
    fprintf(stdout, "server[%p]: %s\n", server, buf);

    zmq_send(server, identity, strlen(identity), ZMQ_SNDMORE);
    zmq_send(server, "", 0, ZMQ_SNDMORE);
    zmq_send(server, "OK", 2, 0);
  }

  zmq_close(server);
  zmq_ctx_destroy(ctx);

  return NULL;
}



int 
start_server(int num_server)
{
  int i;
  for (i = 0; i < num_server; ++i) {
    pthread_t tid;
    pthread_create(&tid, NULL, server_routine, NULL);
  }

  return 0;
}
