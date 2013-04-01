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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <czmq.h>


int 
main(int argc, char* argv[])
{
  char* self;
  zctx_t* ctx;
  int i;
  void* statebe;
  void* statefe;

  if (argc < 2) {
    fprintf(stderr, "syntax: peeing me {you} ...\n");
    return 1;
  }
  self = argv[1];
  fprintf(stdout, "I: preparing broker at %s...\n", self);
  srand((unsigned int)time(NULL));

  ctx = zctx_new();

  statebe = zsocket_new(ctx, ZMQ_PUB);
  zsocket_bind(statebe, "ipc://%s-state.ipc", self);

  statefe = zsocket_new(ctx, ZMQ_SUB);
  zsockopt_set_subscribe(statefe, "");
  for (i = 2; i < argc; ++i) {
    char* peer = argv[i];
    fprintf(stdout, "I: connecting to state backed at '%s'\n", peer);
    zsocket_connect(statefe, "ipc://%s-state.ipc", peer);
  }


  while (1) {
    zmq_pollitem_t items[] = {{statefe, 0, ZMQ_POLLIN, 0}};
    int r = zmq_poll(items, 1, 1000 * ZMQ_POLL_MSEC);
    if (-1 == r)
      break;

    if (items[0].revents & ZMQ_POLLIN) {
      char* peer_name = zstr_recv(statefe);
      char* available = zstr_recv(statefe);
      fprintf(stdout, "%s - %s workers free\n", peer_name, available);
      free(peer_name);
      free(available);
    }
    else {
      int randv = rand() % 10;
      zstr_sendm(statebe, self);
      zstr_sendf(statebe, "%d", randv);
      fprintf(stdout, "sending : %d\n", randv);
    }
  }


  zctx_destroy(&ctx);

  return 0;
}
