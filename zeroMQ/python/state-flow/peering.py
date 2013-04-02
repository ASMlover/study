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

import sys
import time 
import random
import zmq 



def main(myself, others):
  print 'Hello, I am ', myself
  ctx = zmq.Context() 

  statebe = ctx.socket(zmq.PUB)
  statefe = ctx.socket(zmq.SUB)
  statefe.setsockopt(zmq.SUBSCRIBE, '')

  bind_address = 'ipc://%s-state.ipc' % myself
  statebe.bind(bind_address) 

  for other in others:
    statefe.connect('ipc://%s-state.ipc' % other)
    time.sleep(1.0) 
  
  poller = zmq.Poller()
  poller.register(statefe, zmq.POLLIN) 

  while True:
    socks = dict(poller.poll(1000))
    try:
      if socks[statefe] == zmq.POLLIN:
        msg = statefe.recv_multipart()
        print 'received: %s' % msg
    except KeyError:
      msg = [bind_address, str(random.randint(1, 10))]
      statebe.send_multipart(msg)
  
  poller.unregister(statefe)
  time.sleep(1.0)


if __name__ == '__main__':
  if len(sys.argv) >= 2:
    main(myself=sys.argv[1], others=sys.argv[2:])
  else:
    print 'Usage: peering.py <myself> <peer_1> ... <peer_n>'
    exit(1)
