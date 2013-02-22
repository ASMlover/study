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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>



int 
main(int argc, char* argv[])
{
  void* ctx = zmq_ctx_new();
  void* pub = zmq_socket(ctx, ZMQ_PUB);
  void* sync = zmq_socket(ctx, ZMQ_REP);

  zmq_bind(pub, "tcp://*:5555");
  zmq_bind(sync, "tcp://*:6666");

  fprintf(stdout, "waiting for subscribers\n");
  do {
    int i, num_sub, num_pub;
    char buf[128];
    if (argc < 3) {
      fprintf(stderr, "arguments error ...\n");
      break;
    }
    num_sub = atoi(argv[1]);
    num_pub = atoi(argv[2]);
    fprintf(stdout, "num_sub=>%d, num_pub=>%d\n", num_sub, num_pub);

    for (i = 0; i < num_sub; ++i) {
      zmq_recv(sync, buf, sizeof(buf), 0);
      zmq_send(sync, "", 0, 0);
    }

    fprintf(stdout, "broadcasting messages ...\n");
    for (i = 0; i < num_pub; ++i) {
      sprintf(buf, "publish index : %d", i + 1);
      zmq_send(pub, buf, strlen(buf), 0);
      fprintf(stdout, "sent : %s\n", buf);
    }
    zmq_send(pub, "END", 3, 0);
    fprintf(stdout, "sent : END ...\n");
  } while (0);

  zmq_close(sync);
  zmq_close(pub);
  zmq_ctx_destroy(ctx);

  return 0;
}
