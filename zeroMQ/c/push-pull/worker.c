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
#include <unistd.h>


int 
main(int argc, char* argv[])
{
  char buf[32];
  int  work_time;
  void* ctx = zmq_ctx_new();
  void* worker = zmq_socket(ctx, ZMQ_PULL);
  void* sinker = zmq_socket(ctx, ZMQ_PUSH);
  zmq_connect(worker, "tcp://localhost:5555");
  zmq_connect(sinker, "tcp://localhost:6666");

  fprintf(stdout, "worker listener is ready ...\n");
  while (1) {
    memset(buf, 0, sizeof(buf));
    zmq_recv(worker, buf, sizeof(buf), 0);
    work_time = atoi(buf);
    fprintf(stdout, "begin to work on task use '%d ms' ===>\n", work_time);
    usleep(work_time * 1000);
    fprintf(stdout, "\tfinished the task ...\n");

    sprintf(buf, "task[%d ms] end", work_time);
    zmq_send(sinker, buf, strlen(buf), 0);
  }

  zmq_close(sinker);
  zmq_close(worker);
  zmq_ctx_destroy(ctx);

  return 0;
}
