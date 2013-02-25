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
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <zmq.h>



static int64_t 
s_clock(void) 
{
#if (defined(__WINDOWS__))
  SYSTEMTIME s;
  GetLocalTime(&s);
  return (int64_t)s.wSecond * 1000 + s.wMilliseconds;
#else 
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (int64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}


static void* 
worker_routine(void* arg)
{
  uint32_t tid = (uint32_t)pthread_self();
  int total = 0;
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* worker = zmq_socket(ctx, ZMQ_REQ);

  sprintf(buf, "%08X", tid);
  zmq_setsockopt(worker, ZMQ_IDENTITY, buf, strlen(buf));
  zmq_connect(worker, "tcp://localhost:5555");

  fprintf(stdout, "thread worker[%s] init success ...\n", buf);

  srand((unsigned int)time(0));
  while (1) {
    sprintf(buf, "Hi Boss");
    zmq_send(worker, buf, strlen(buf), 0);
  
    memset(buf, 0, sizeof(buf));
    zmq_recv(worker, buf, sizeof(buf), 0);
    if (0 == strcmp("Fired!", buf)) {
      fprintf(stdout, "worker[%08X] Completed: %d tasks\n", tid, total);
      break;
    }
    ++total;
    
    usleep(rand() % 1000 * 1000);
  }

  zmq_close(worker);
  zmq_ctx_destroy(ctx);

  return NULL;
}


int 
main(int argc, char* argv[])
{
  int i, workers;
  char buf[128];
  void* ctx = zmq_ctx_new();
  void* broker = zmq_socket(ctx, ZMQ_ROUTER);

  zmq_bind(broker, "tcp://*:5555");

  do {
    int end_time, workers_fired = 0;
    if (argc < 2) {
      fprintf(stdout, "arguments error ...\n");
      break;
    }
    workers = atoi(argv[1]);

    for (i = 0; i < workers; ++i) {
      pthread_t tid;
      pthread_create(&tid, NULL, worker_routine, NULL);
    }

    end_time = s_clock() + 5000;
    while (1) {
      memset(buf, 0, sizeof(buf));
      zmq_recv(broker, buf, sizeof(buf), 0);
      zmq_send(broker, buf, strlen(buf), ZMQ_SNDMORE);
      zmq_recv(broker, buf, sizeof(buf), 0);
      zmq_recv(broker, buf, sizeof(buf), 0);
      zmq_send(broker, "", 0, ZMQ_SNDMORE);

      if (s_clock() < end_time) {
        sprintf(buf, "wrok harder");
        zmq_send(broker, buf, strlen(buf), 0);
      } 
      else {
        sprintf(buf, "Fired!");
        zmq_send(broker, buf, strlen(buf), 0);
        if (++workers_fired == workers)
          break;
      }
    }
  } while (0);

  zmq_close(broker);
  zmq_ctx_destroy(ctx);

  return 0;
}
