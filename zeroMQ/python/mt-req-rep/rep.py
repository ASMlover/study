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
import threading
import time
import sys


def routine(ctx):
  rep = ctx.socket(zmq.REP)
  rep.connect('inproc://workers')

  thread_name = threading.currentThread().getName()
  index = 0
  while True:
    msg = rep.recv()
    print 'recevied request : %s' % msg

    index += 1
    rep.send('[%s]reply request index : %d' % (thread_name, index))
  rep.close()



if __name__ == '__main__':
  if (len(sys.argv) < 2):
    print 'arguments error ...'
    exit()

  ctx = zmq.Context(1)

  rep = ctx.socket(zmq.ROUTER)
  rep.bind('tcp://*:5555')

  worker = ctx.socket(zmq.DEALER)
  worker.bind('inproc://workers')

  print 'reply server init success ...'
  for i in range(int(sys.argv[1])):
    thread = threading.Thread(target=routine, args=(ctx,))
    thread.start()
  zmq.device(zmq.QUEUE, rep, worker)

  worker.close()
  rep.close()
  ctx.term()
