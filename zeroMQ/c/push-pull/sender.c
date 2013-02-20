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
#include <zmq.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void 
start_sender(int num_task)
{
  int i, work_time, total_time = 0;
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* sender = zmq_socket(ctx, ZMQ_PUSH);
  void* sinker = zmq_socket(ctx, ZMQ_PUSH);
  zmq_bind(sender, "tcp://*:5555");
  zmq_connect(sinker, "tcp://localhost:6666");

  fprintf(stdout, 
    "sender server init success ...\n"
    "wait for workers were readied (presss ENTER) : ");
  getchar();

  strcpy(buf, "now send tasks to workers ...");
  zmq_send(sinker, buf, strlen(buf), 0);

  srand((unsigned int)time(0));
  for (i = 0; i < num_task; ++i) {
    work_time = rand() % 5000;
    work_time += (0 == work_time ? 1 : 0);
    total_time += work_time;

    sprintf(buf, "%d", work_time);
    zmq_send(sender, buf, strlen(buf), 0);
  }
  fprintf(stdout, "all tasks will use : %d ms\n", total_time);

  zmq_close(sinker);
  zmq_close(sender);
  zmq_ctx_destroy(ctx);
}


int 
main(int argc, char* argv[])
{
  if (argc < 2)
    return fprintf(stderr, "argument error ...\n");

  start_sender(atoi(argv[1]));

  return 0;
}
