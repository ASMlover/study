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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int 
main(int argc, char* argv[])
{
  void* ctx = zmq_ctx_new();
  void* rep = zmq_socket(ctx, ZMQ_REP);
  zmq_connect(rep, "tcp://localhost:6666");

  do {
    int count = 0, server_id;
    char buf[128];

    if (argc < 2) {
      fprintf(stderr, "arguments error ...\n");
      break;
    }
    server_id = atoi(argv[1]);

    fprintf(stdout, "reply server[%d] init success ...\n", server_id);
    while (1) {
      memset(buf, 0, sizeof(buf));
      zmq_recv(rep, buf, sizeof(buf), 0);
      fprintf(stdout, "recevied request {%s}\n", buf);

      sprintf(buf, "[%d] reply [%d]\n", server_id, ++count);
      zmq_send(rep, buf, strlen(buf), 0);
    }
  } while (0);

  zmq_close(rep);
  zmq_ctx_destroy(ctx);

  return 0;
}
