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
#include <string.h>
#include <stdint.h>
#include <zmq.h>



static void 
s_dump(void* socket)
{
  int i, size, is_text = 1;
  unsigned char* data;
  zmq_msg_t msg;
  int64_t more;
  size_t more_size;
  fprintf(stdout, "---------------------------------\n");
  while (1) {
    is_text = 1;
    zmq_msg_init(&msg);
    size = zmq_msg_recv(&msg, socket, 0);
    data = (unsigned char*)zmq_msg_data(&msg);
    for (i = 0; i < size; ++i) {
      if (data[i] < 32 || data[i] > 127)
        is_text = 0;
    }

    fprintf(stdout, "[%03d] ", size);
    for (i = 0; i < size; ++i) {
      if (is_text)
        fprintf(stdout, "%c", data[i]);
      else 
        fprintf(stdout, "%02X", data[i]);
    }
    fprintf(stdout, "\n");

    more = 0;
    more_size = sizeof(more);
    zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
    zmq_msg_close(&msg);

    if (!more)
      break;
  }
}



int 
main(int argc, char* argv[])
{
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* sink = zmq_socket(ctx, ZMQ_ROUTER);
  void* no_id = zmq_socket(ctx, ZMQ_REQ);
  void* with_id = zmq_socket(ctx, ZMQ_REQ);

  zmq_bind(sink, "inproc://example");
  
  zmq_connect(no_id, "inproc://example");
  sprintf(buf, "ROUTER uses a random generated UUID");
  zmq_send(no_id, buf, strlen(buf), 0);
  s_dump(sink);

  zmq_setsockopt(with_id, ZMQ_IDENTITY, "with_id", 7);
  zmq_connect(with_id, "inproc://example");
  sprintf(buf, "ROUTER uses REQ's indentity");
  zmq_send(with_id, buf, strlen(buf), 0);
  s_dump(sink);

  zmq_close(with_id);
  zmq_close(no_id);
  zmq_close(sink);
  zmq_ctx_destroy(ctx);

  return 0;
}
