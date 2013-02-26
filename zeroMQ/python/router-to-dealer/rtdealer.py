#!/usr/bin/python 
# -*- coding: utf-8 -*-
#
# Copyright (c) 2013 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import time
import sys
import threading
import random
import zmq 



def work_routine():
  ctx = zmq.Context()

  worker = ctx.socket(zmq.DEALER)
  worker.setsockopt(zmq.IDENTITY, 'A')
  worker.connect('tcp://localhost:5555')

  print 'worker[A] init success ...'
  random.seed()
  total = 0
  while True:
    try:
      request = worker.recv()
      if request == 'END':
        print 'worker[A] completed %d tasks' % total
        break
      total += 1
      time.sleep(random.randint(1, 100) * 0.001)
    except KeyboardInterrupt:
      break

  worker.close() 





if __name__ == '__main__':
  ctx = zmq.Context()
  broker = ctx.socket(zmq.ROUTER)
  broker.bind('tcp://*:5555')
  
  thread = threading.Thread(target=work_routine, args=())
  thread.start()

  time.sleep(1)
  print 'broker init success ...'
  for _ in range(10):
    broker.send('A', zmq.SNDMORE)
    broker.send('this is the workload')

  broker.send('A', zmq.SNDMORE)
  broker.send('END')

  broker.close()
