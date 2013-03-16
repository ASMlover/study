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
import threading
import time 
import zmq

from zmq.eventloop.ioloop import IOLoop 
from zmq.eventloop.zmqstream import ZMQStream 



def client_routine(client_url, i):
  ctx = zmq.Context()
  client = ctx.socket(zmq.REQ)
  identity = 'client-%d' % i
  client.setsockopt(zmq.IDENTITY, identity)
  client.connect(client_url)
  client.send('Hello')
  reply = client.recv()
  print '%s: %s' % (identity, reply)
  client.close()


def worker_routine(worker_url, i):
  ctx = zmq.Context()
  worker = ctx.socket(zmq.REQ)
  identity = 'worker-%d' % i
  worker.setsockopt(zmq.IDENTITY, identity)
  worker.connect(worker_url)
  worker.send('READY')
  try:
    while True:
      address, empty, request = worker.recv_multipart()
      print '%s: %s' % (identity, request)
      worker.send_multipart([address, '', 'OK'])
  except zmq.ZMQError, err:
    if err.strerror == 'Context was terminated':
      return
    else:
      raise err
  worker.close()


class LRUQueue(object):
  def __init__(self, backend_socket, frontend_socket, clients, workers):
    self.avaliable_workers = 0
    self.workers = []
    self.worker_num = workers
    self.client_num = clients
    self.backend = ZMQStream(backend_socket)
    self.frontend = ZMQStream(frontend_socket)
    self.backend.on_recv(self.handle_backend)
    self.loop = IOLoop.instance()
 
  def handle_backend(self, msg):
    worker_addr, empty, client_addr = msg[:3]
    assert self.avaliable_workers < self.worker_num 
    self.avaliable_workers += 1
    self.workers.append(worker_addr)
    assert empty == ''
    if client_addr != 'READY':
      empty, reply = msg[3:]
      assert empty == ''
      self.frontend.send_multipart([client_addr, '', reply])
      self.client_num -= 1
      if 0 == self.client_num:
        self.loop.add_timeout(time.time() + 1, self.loop.stop)
    if self.avaliable_workers == 1:
      self.frontend.on_recv(self.handle_frontend)

  def handle_frontend(self, msg):
    client_addr, empty, request = msg
    assert empty == ''
    self.avaliable_workers -= 1
    worker_id = self.workers.pop()
    self.backend.send_multipart([worker_id, '', client_addr, '', request])
    if self.avaliable_workers == 0:
      self.frontend.stop_on_recv()



def main(client_num, worker_num):
  client_url = 'ipc://frontend.ipc'
  worker_url = 'ipc://backend.ipc'

  ctx = zmq.Context()
  frontend = ctx.socket(zmq.ROUTER)
  frontend.bind(client_url)
  backend = ctx.socket(zmq.ROUTER)
  backend.bind(worker_url)

  for i in range(worker_num):
    t = threading.Thread(target=worker_routine, args=(worker_url, i))
    t.daemon = True
    t.start()
  for i in range(client_num):
    t = threading.Thread(target=client_routine, args=(client_url, i))
    t.daemon = True
    t.start()
  
  queue = LRUQueue(backend, frontend, client_num, worker_num)
  IOLoop.instance().start()

  backend.close()
  frontend.close()



if __name__ == '__main__':
  if (len(sys.argv) < 3):
    print 'argument error ...'
    exit()
  main(int(sys.argv[1]), int(sys.argv[2]))
