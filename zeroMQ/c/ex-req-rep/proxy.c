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


int 
main(int argc, char* argv[])
{
  size_t more, more_size;
  zmq_msg_t msg;
  void* ctx = zmq_ctx_new();
  void* req = zmq_socket(ctx, ZMQ_ROUTER);
  void* rep = zmq_socket(ctx, ZMQ_DEALER);
  zmq_pollitem_t items[] = {
    { req, 0, ZMQ_POLLIN, 0 }, 
    { rep, 0, ZMQ_POLLIN, 0 },
  };

  zmq_bind(req, "tcp://*:5555");
  zmq_bind(rep, "tcp://*:6666");

  fprintf(stdout, "proxy server init success ...\n");
  while (1) {
    zmq_poll(items, 2, -1);

    if (items[0].revents & ZMQ_POLLIN) {
      while (1) {
        zmq_msg_init(&msg);
        zmq_msg_recv(&msg, req, 0);
        more_size = sizeof(more);
        zmq_getsockopt(req, ZMQ_RCVMORE, &more, &more_size);
        zmq_msg_send(&msg, rep, (more ? ZMQ_SNDMORE : 0));
        zmq_msg_close(&msg);

        if (!more)
          break;
      }
    }

    if (items[1].revents & ZMQ_POLLIN) {
      while (1) {
        zmq_msg_init(&msg);
        zmq_msg_recv(&msg, rep, 0);
        more_size = sizeof(more);
        zmq_getsockopt(rep, ZMQ_RCVMORE, &more, &more_size);
        zmq_msg_send(&msg, req, (more ? ZMQ_SNDMORE : 0));
        zmq_msg_close(&msg);

        if (!more)
          break;
      }
    }
  }

  zmq_close(rep);
  zmq_close(req);
  zmq_ctx_destroy(ctx);

  return 0;
}
