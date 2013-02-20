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
#    notice, this list of conditions and the following disclaimer in
#  * Redistributions in binary form must reproduce the above copyright
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

import zmq
import time


if __name__ == '__main__':
  ctx = zmq.Context()

  worker = ctx.socket(zmq.PULL)
  worker.connect('tcp://localhost:5555')

  sinker = ctx.socket(zmq.PUSH)
  sinker.connect('tcp://localhost:6666')

  controller = ctx.socket(zmq.SUB)
  controller.connect('tcp://localhost:7777')
  controller.setsockopt(zmq.SUBSCRIBE, "")

  poller = zmq.Poller()
  poller.register(worker, zmq.POLLIN)
  poller.register(controller, zmq.POLLIN)

  print 'workers are ready ...'
  while True:
    socks = dict(poller.poll())

    if socks.get(worker) == zmq.POLLIN:
      msg = worker.recv()
      time.sleep(int(msg) * 0.001)
      sinker.send('task finished used `%s ms`' % msg)
      print 'finish this task used %s ms' % msg
    
    if socks.get(controller) == zmq.POLLIN:
      break
