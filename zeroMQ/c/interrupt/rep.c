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
#include <signal.h>
#include <string.h>


static int s_interrupted = 0;
static void 
s_signal_handler(int signal_value)
{
  s_interrupted = 1;
}

static void 
s_catch_signals(void)
{
  struct sigaction action;

  action.sa_handler = s_signal_handler;
  action.sa_flags   = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
}


int 
main(int argc, char* argv[])
{
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* rep = zmq_socket(ctx, ZMQ_REP);
  zmq_bind(rep, "tcp://*:5555");

  fprintf(stdout, "server init success ...\n");
  s_catch_signals();
  while (1) {
    memset(buf, 0, sizeof(buf));
    zmq_recv(rep, buf, sizeof(buf), 0);

    if (s_interrupted) {
      fprintf(stdout, "W: interrupt recevied, killing server ...\n");
      break;
    }
  }

  zmq_close(rep);
  zmq_ctx_destroy(ctx);

  return 0;
}
