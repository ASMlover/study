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
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
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


static void* 
routine_step1(void* ctx)
{
  char buf[128];
  void* step1 = zmq_socket(ctx, ZMQ_PAIR);
  zmq_connect(step1, "inproc://step2");

  sprintf(buf, "step1 ready, signaling step2");
  zmq_send(step1, buf, strlen(buf), 0);

  zmq_close(step1);

  return NULL;
}


static void* 
routine_step2(void* ctx)
{
  char buf[128] = {0};
  pthread_t pid;
  void* step2 = zmq_socket(ctx, ZMQ_PAIR);
  void* step3 = zmq_socket(ctx, ZMQ_PAIR);

  zmq_bind(step2, "inproc://step2");
  pthread_create(&pid, NULL, routine_step1, ctx);
  zmq_recv(step2, buf, sizeof(buf), 0);
  fprintf(stdout, "step2 recevied : %s\n", buf);

  zmq_connect(step3, "inproc://step3");
  sprintf(buf, "step2 ready, signaling step3");
  zmq_send(step3, buf, strlen(buf), 0);

  zmq_close(step3);
  zmq_close(step2);

  return NULL;
}


int 
main(int argc, char* argv[])
{
  char buf[128] = {0};
  void* ctx = zmq_ctx_new();
  void* step3 = zmq_socket(ctx, ZMQ_PAIR);
  zmq_bind(step3, "inproc://step3");
  pthread_t pid;

  pthread_create(&pid, NULL, routine_step2, ctx);

  zmq_recv(step3, buf, sizeof(buf), 0);
  fprintf(stdout, "step3 recevied : %s\n", buf);

  fprintf(stdout, "test successful ...\n");
  zmq_close(step3);
  zmq_ctx_destroy(ctx);

  return 0;
}
