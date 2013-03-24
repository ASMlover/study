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
import zmq
import threading 
import time 
import random 



class ClientTask(threading.Thread):
  ''' Task of Client '''
  def __init__(self):
    threading.Thread.__init__(self)

  def run(self):
    ctx = zmq.Context()
    socket = ctx.socket(zmq.DEALER)
    identity = 'worker-%d' % random.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
    socket.setsockopt(zmq.IDENTITY, identity)
    socket.connect('tcp://localhost:5555')
    print 'client % started' % identity 
    poll = zmq.Poller()
    poll.register(socket, zmq.POLLIN)
    request = 0
    while True:
      for i in range(5):
        socks = dict(poll.poll(1000))
        if socket in socks:
          if socks[socket] == zmq.POLLIN:
            msg = socket.recv()
            print 'Client %s received: %s' % (identity, msg)
            del msg 
      request = request + 1
      print 'request #%d sent ...' % request
      socket.send('request #%d' % request)

    socket.close()
    ctx.term() 




class ServerTask(threading.Thread):
  ''' Task of Server '''
  def __init__(self):
    threading.Thread.__init__(self)

  def run(self):
    ctx = zmq.Context()
    frontend = ctx.socket(zmq.ROUTER)
    frontend.bind('tcp://*:5555')

    backend = ctx.socket(zmq.DEALER)
    backend.bind('inproc://backend')

    workers = []
    for i in range(5):
      worker = ServerWorker(ctx)
      worker.start()
      workers.append(worker)
    poll = zmq.Poller()
    poll.register(frontend, zmq.POLLIN)
    poll.register(backend, zmq.POLLIN)
    while True:
      socks = dict(poll.poll())
      if frontend in socks:
        if socks[frontend] == zmq.POLLIN:
          identity = frontend.recv()
          msg = frontend.recv()
          print 'Server received %s id %s' % (msg, identity)
          backend.send(identity, zmq.SNDMORE)
          backend.send(msg)
      if backend in socks:
        if socks[backend] == zmq.POLLIN:
          identity = backend.recv()
          msg = backend.recv()
          print 'Sending to frontend %s id %s' % (msg, identity)
          frontend.send(identity, zmq.SNDMORE)
          frontend.send(msg)
    frontend.close()
    backend.close()
    ctx.term()



class ServerWorker(threading.Thread):
  ''' worker of Server '''
  def __init__(self, ctx):
    threading.Thread.__init__(self)
    self.ctx = ctx 

  def run(self):
    worker = self.ctx.socket(zmq.DEALER)
    worker.connect('inproc://backend')
    while True:
      identity = worker.recv()
      msg = worker.recv()
      print 'worker received %s from %s' % (msg, identity)
      replies = random.choice(range(5))
      for i in range(replies):
        time.sleep(1 / random.choice(range(1, 10)))
        worker.send(identity, zmq.SNDMORE)
        worker.send(msg)
      del msg 
    worker.close()




def main():
  server = ServerTask()
  server.start()
  for i in range(3):
    client = ClientTask()
    client.start()
  
  time.sleep(5)



if __name__ == '__main__':
  main()
