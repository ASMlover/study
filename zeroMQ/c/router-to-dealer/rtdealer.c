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
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <utils.h>



static void* 
worker_routine(void* arg)
{
  uint32_t thread_id = (uint32_t)pthread_self();
  void* ctx;
  void* worker;
  char identity[16];
  char buf[128];
  int total = 0;

  ctx = zmq_ctx_new();

  worker = zmq_socket(ctx, ZMQ_DEALER);
  sprintf(identity, "%08X", thread_id);
  zmq_setsockopt(worker, ZMQ_IDENTITY, identity, strlen(identity));
  zmq_connect(worker, "tcp://localhost:5555");

  fprintf(stdout, "worker[%s] init success ...\n", identity);
  srand((unsigned int)time(0));
  while (1) {
    zmq_send(worker, "", 0, ZMQ_SNDMORE);
    sprintf(buf, "Hi Boss");
    zmq_send(worker, buf, strlen(buf), 0);
    zmq_recv(worker, buf, sizeof(buf), 0);

    memset(buf, 0, sizeof(buf));
    zmq_recv(worker, buf, sizeof(buf), 0);
    if (0 == strcmp("Fired!", buf)) {
      fprintf(stdout, "worker[%s] completed: %d tasks\n", identity, total);
      break;
    }

    ++total;
    usleep(rand() % 100 * 100);
  }

  zmq_close(worker);
  zmq_ctx_destroy(ctx);

  return NULL;
}

int 
main(int argc, char* argv[])
{
  void* ctx = NULL; 
  void* broker;
  int i, num_workers;
  int end_time, workers_fired = 0;
  char buf[128];
  char identity[16];

  if (argc < 2) {
    fprintf(stderr, "arguments error ...\n");
    return -1;
  }
  num_workers = atoi(argv[1]);
  
  ctx = zmq_ctx_new();

  broker = zmq_socket(ctx, ZMQ_ROUTER);
  zmq_bind(broker, "tcp://*:5555");

  for (i = 0; i < num_workers; ++i) {
    pthread_t tid;
    pthread_create(&tid, NULL, worker_routine, NULL);
  }

  end_time = get_clock() + 5000;
  while (1) {
    memset(identity, 0, sizeof(identity));
    zmq_recv(broker, identity, sizeof(identity), 0);
    zmq_send(broker, identity, strlen(identity), ZMQ_SNDMORE);
    zmq_recv(broker, buf, sizeof(buf), 0);
    zmq_recv(broker, buf, sizeof(buf), 0);
    zmq_send(broker, "", 0, ZMQ_SNDMORE);

    if (get_clock() < end_time) {
      sprintf(buf, "work harder");
      zmq_send(broker, buf, strlen(buf), 0);
    }
    else {
      sprintf(buf, "Fired!");
      zmq_send(broker, buf, strlen(buf), 0);
      if (num_workers == ++workers_fired)
        break;
    }
  }

  zmq_close(broker);
  zmq_ctx_destroy(ctx);

  return 0;
}
