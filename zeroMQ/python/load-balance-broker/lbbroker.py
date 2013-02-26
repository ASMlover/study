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



def client_routine(client_url, ctx, idx):
  client = ctx.socket(zmq.REQ)
  identity = 'client-%d' % idx
  client.setsockopt(zmq.IDENTITY, identity)
  client.connect(client_url)

  client.send('HELLO')
  reply = client.recv()
  print '%s: %s\n' % (identity, reply)

  client.close()




def server_routine(server_url, ctx, idx):
  server = ctx.socket(zmq.REQ)
  identity = 'server-%d' % idx
  server.setsockopt(zmq.IDENTITY, identity)
  server.connect(server_url)

  server.send('READY')
  print '%s READY' % identity
  try:
    while True:
      address = server.recv()
      empty = server.recv()
      request = server.recv()

      print "%s: %s" % (identity, request)

      server.send(address, zmq.SNDMORE)
      server.send("", zmq.SNDMORE)
      server.send("OK")
  except zmq.ZMQError, err:
    if err.strerror == 'Context was terminated':
      return
    else:
      return err 
  
  server.close()






def broker_main(num_client, num_server):
  client_url = 'inproc://client'
  server_url = 'inproc://server'

  ctx = zmq.Context(1)
  client = ctx.socket(zmq.ROUTER)
  client.bind(client_url)

  server = ctx.socket(zmq.ROUTER)
  server.bind(server_url)

  for i in range(num_server):
    thread = threading.Thread(target=server_routine, 
        args=(server_url, ctx, i))
    thread.start()

  for i in range(num_client):
    thread = threading.Thread(target=client_routine, 
        args=(client_url, ctx, i))
    thread.start()
 
  
  available_workers = 0
  workers_queue = []

  poll = zmq.Poller()
  poll.register(server, zmq.POLLIN)
  poll.register(client, zmq.POLLIN)

  while True:
    socks = dict(poll.poll())

    if (server in socks and socks[server] == zmq.POLLIN):
      address = server.recv()
      available_workers += 1
      workers_queue.append(address)
      empty = server.recv()

      client_addr = server.recv()
      if client_addr != 'READY':
        empty = server.recv()
        reply = server.recv()

        client.send(client_addr, zmq.SNDMORE)
        client.send("", zmq.SNDMORE)
        client.send(reply)

        num_client -= 1
        if 0 == num_client:
          break
    if available_workers > 0:
      if (client in socks and socks[client] == zmq.POLLIN):
        client_addr = client.recv()
        empty = client.recv()
        request = client.recv()

        available_workers -= 1
        server_id = workers_queue.pop()

        server.send(server_id, zmq.SNDMORE)
        server.send("", zmq.SNDMORE)
        server.send(client_addr, zmq.SNDMORE)
        server.send("", zmq.SNDMORE)
        server.send(request)
  
  time.sleep(1)

  server.close()
  client.close()
  ctx.term()





if __name__ == '__main__':
  if (len(sys.argv) < 3):
    print 'arguments error ...'
    exit()
  
  num_client = int(sys.argv[1])
  num_server = int(sys.argv[2])
  broker_main(num_client, num_server)
