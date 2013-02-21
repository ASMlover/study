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
import signal


interrupted = False
def signal_handler(signum, frame):
  global interrupted
  interrupted = True


if __name__ == '__main__':
  ctx = zmq.Context()
  rep = ctx.socket(zmq.REP)
  rep.bind('tcp://*:5555')

  print 'reply init sucess ...'
  try:
    rep.recv()
  except KeyboardInterrupt:
    print 'W: interrupte received, proceeding ...'
  
  count = 0
  signal.signal(signal.SIGINT, signal_handler)
  while True:
    try:
      msg = rep.recv(zmq.DONTWAIT)
    except zmq.ZMQError:
      pass
    count += 1
    if interrupted:
      print 'W: interrupte received, Killing server ...'
      break

  rep.close()
