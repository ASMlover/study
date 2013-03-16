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
import threading 
import zmq


def client_routine(client_url, ctx, i):
  client = ctx.socket(zmq.REQ);
  identity = 'client-%d' % i
  client.setsockopt(zmq.IDENTITY, identity)
  client.connect(client_url)
  client.send("Hello")
  reply = client.recv()
  print '%s: %s' % (identity, reply)
  return 


def worker_routine(worker_url, ctx, i):
  worker = ctx.socket(zmq.REQ)
  identity = 'worker-%d' % i
  worker.setsockopt(zmq.IDENTITY, identity)
  worker.connect(worker_url)
  worker.send('READY')
  try:
    while True:
      [address, empty, request] = worker.recv_multipart()
      print '%s: %s' % (identity, request)
      worker.send_multipart([address, '', 'OK'])
  except zmq.ZMQError, err:
    if err.strerror == 'Context was terminated':
      return 
    else:
      raise err


def main(client_num, worker_num):
  client_url = 'inproc://client'
  worker_url = 'inproc://worker'

  ctx = zmq.Context(1)
  frontend = ctx.socket(zmq.ROUTER)
  frontend.bind(client_url)
  backend = ctx.socket(zmq.ROUTER)
  backend.bind(worker_url)

  for i in range(worker_num):
    t = threading.Thread(target=worker_routine, args=(worker_url, ctx, i))
    t.start()
  for i in range(client_num):
    t = threading.Thread(target=client_routine, args=(client_url, ctx, i))
    t.start()

  available_workers = 0
  workers_list = []
  poller = zmq.Poller()
  poller.register(backend, zmq.POLLIN)
  poller.register(frontend, zmq.POLLIN)
  while True:
    socks = dict(poller.poll())
    if backend in socks and socks[backend] == zmq.POLLIN:
      msg = backend.recv_multipart()
      assert available_workers < worker_num 
      worker_address = msg[0]
      available_workers += 1
      workers_list.append(worker_address)
      empty = msg[1]
      assert empty == ''
      client_address = msg[2]
      if client_address != 'READY':
        empty = msg[3]
        assert empty == ''
        reply = msg[4]
        frontend.send_multipart([client_address, '', reply])
        client_num -= 1
        if client_num == 0:
          break
    if available_workers > 0:
      if frontend in socks and socks[frontend] == zmq.POLLIN:
        [client_address, empty, request] = frontend.recv_multipart()
        assert empty == ''
        available_workers -= 1
        worker_id = workers_list.pop()
        backend.send_multipart([worker_id, '', client_address, '', request])
  
  time.sleep(1)
  frontend.close()
  backend.close()
  ctx.term()



if __name__ == '__main__':
  if (len(sys.argv) < 3):
    print 'arguments error ...'
    exit()
  
  main(int(sys.argv[1]), int(sys.argv[2]))
