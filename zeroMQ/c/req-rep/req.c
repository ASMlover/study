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
#if (defined(_WIN32) || defined(_WIN64))
  #include <windows.h>
  #define __sleep     Sleep
#else
  #include <unistd.h>
  #define __sleep(m)  usleep(m * 1000)
#endif


int 
main(int argc, char* argv[])
{
  int i;
  void* ctx = zmq_ctx_new();
  void* s = zmq_socket(ctx, ZMQ_REQ);
  zmq_connect(s, "tcp://localhost:5555");

  for (i = 0; i < 10; ++i) {
    zmq_msg_t req, rep;

    zmq_msg_init_size(&req, 128);
    sprintf((char*)zmq_msg_data(&req), 
      "Hello, world! current count[%d]", i);
    zmq_msg_send(&req, s, 0);
    zmq_msg_close(&req);

    zmq_msg_init(&rep);
    zmq_msg_recv(&rep, s, 0);
    fprintf(stdout, "reply message : %s\n", (char*)zmq_msg_data(&rep));
    zmq_msg_close(&rep);

    __sleep(100);
  }

  zmq_close(s);
  zmq_ctx_destroy(ctx);

  return 0;
}
