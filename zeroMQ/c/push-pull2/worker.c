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
#include <zmq.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int 
main(int argc, char* argv[])
{
  int task_time;
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* worker = zmq_socket(ctx, ZMQ_PULL);
  void* sinker = zmq_socket(ctx, ZMQ_PUSH);
  void* controller = zmq_socket(ctx, ZMQ_SUB);
  zmq_pollitem_t items[] = {
    { worker, 0, ZMQ_POLLIN, 0 }, 
    { controller, 0, ZMQ_POLLIN, 0 }, 
  };

  zmq_connect(worker, "tcp://localhost:5555");
  zmq_connect(sinker, "tcp://localhost:6666");
  zmq_connect(controller, "tcp://localhost:7777");
  zmq_setsockopt(controller, ZMQ_SUBSCRIBE, "", 0);

  fprintf(stdout, "workers are ready ...\n");
  while (1) {
    zmq_poll(items, 2, -1);
    if (items[0].revents & ZMQ_POLLIN) {
      memset(buf, 0, sizeof(buf));
      zmq_recv(worker, buf, sizeof(buf), 0);

      task_time = atoi(buf);
      sprintf(buf, "this task worked `%d ms`", task_time);
      fprintf(stdout, "%s\n", buf);
      usleep(task_time * 1000);
      zmq_send(sinker, buf, strlen(buf), 0);
    } 

    if (items[1].revents & ZMQ_POLLIN) 
      break;
  }

  zmq_close(controller);
  zmq_close(sinker);
  zmq_close(worker);
  zmq_ctx_destroy(ctx);

  return 0;
}
