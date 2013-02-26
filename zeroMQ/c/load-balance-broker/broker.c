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
#include <string.h>
#include <zmq.h>

#include "lbbroker.h"




#define DEQUE(q) memmove(&(q)[0], &(q)[1], sizeof(q) - sizeof(q[0]))



int 
start_broker(int num_client, int num_server)
{
  int available_workers = 0;
  char* work_queue[10];
  void* ctx = zmq_ctx_new();
  void* front = zmq_socket(ctx, ZMQ_ROUTER);
  void* back = zmq_socket(ctx, ZMQ_ROUTER);

  zmq_bind(front, "ipc://front.ipc");
  zmq_bind(back, "ipc://back.ipc");

  start_client(num_client);
  start_server(num_server);

  while (1) {
    zmq_pollitem_t items[] = {
      { back, 0, ZMQ_POLLIN, 0 }, 
      { front, 0, ZMQ_POLLIN, 0 },
    };
    int ret = zmq_poll(items, available_workers ? 2 : 1, -1);
    if (-1 == ret)
      break;

    if (items[0].revents & ZMQ_POLLIN) {
      char client_id[16] = {0};
      char buf[128] = {0};
      char* server_id = (char*)calloc(16, 0);
      zmq_recv(back, server_id, 16, 0);
      work_queue[available_workers++] = server_id;

      zmq_recv(back, buf, sizeof(buf), 0);
      zmq_recv(back, client_id, sizeof(client_id), 0);

      if (0 != strcmp("READY", client_id)) {
        zmq_recv(back, buf, sizeof(buf), 0);
        
        memset(buf, 0, sizeof(buf));
        zmq_recv(back, buf, sizeof(buf), 0);
        zmq_send(front, client_id, strlen(client_id), ZMQ_SNDMORE);
        zmq_send(front, "", 0, ZMQ_SNDMORE);
        zmq_send(front, buf, strlen(buf), 0);
        if (0 == --num_client)
          break;
      }
    }

    if (items[1].revents & ZMQ_POLLIN) {
      char client_id[16] = {0};
      char buf[128];
      zmq_recv(front, client_id, sizeof(client_id), 0);
      zmq_recv(front, buf, sizeof(buf), 0);
      memset(buf, 0, sizeof(buf));
      zmq_recv(front, buf, sizeof(buf), 0);

      zmq_send(back, work_queue[0], strlen(work_queue[0]), ZMQ_SNDMORE);
      zmq_send(back, "", 0, ZMQ_SNDMORE);
      zmq_send(back, client_id, strlen(client_id), ZMQ_SNDMORE);
      zmq_send(back, "", 0, ZMQ_SNDMORE);
      zmq_send(back, buf, strlen(buf), 0);

      free(work_queue[0]);
      DEQUE(work_queue);
      --available_workers;
    }
  }

  zmq_close(back);
  zmq_close(front);
  zmq_ctx_destroy(ctx);

  return 0;
}
