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

import threading
import zmq 



def routine_step1(ctx):
  step1 = ctx.socket(zmq.PAIR)
  step1.connect('inproc://step2')

  step1.send('step1 ready, signaling step2')

  step1.close()


def routine_step2(ctx):
  step2 = ctx.socket(zmq.PAIR)
  step2.bind('inproc://step2')

  thread = threading.Thread(target=routine_step1, args=(ctx,))
  thread.start()

  msg = step2.recv()
  print 'step2 recevied : %s' % msg

  step2.close()

  step3 = ctx.socket(zmq.PAIR)
  step3.connect('inproc://step3')
  
  step3.send('step2 ready, signaling step3')

  step3.close()



def start_main():
  ctx = zmq.Context(1)
  step3 = ctx.socket(zmq.PAIR)
  step3.bind('inproc://step3')

  thread = threading.Thread(target=routine_step2, args=(ctx,))
  thread.start()

  msg = step3.recv()
  print 'step3 recevied : %s' % msg
  print 'testing successful ...'

  step3.close()
  ctx.term()



if __name__ == '__main__':
  start_main()
