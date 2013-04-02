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
import time 
import random
import threading

import zmq 


NUM_CLIENTS = 10
NUM_WORKERS = 3



def client_routine(name, i):
  ctx = zmq.Context()
  client = ctx.socket(zmq.REQ)
  client.identity = 'client-%s-%s' % (name, i)
  client.connect('ipc://%s-localfe.ipc' % name)
  while True:
    client.send("HELLO")
    try:
      reply = client.recv()
    except zmq.ZMQError:
      return
    print 'client-%s: %s' % (i, reply)
    time.sleep(1.0)


def worker_routine(name, i):
  ctx = zmq.Context()
  worker = ctx.socket(zmq.REQ)
  worker.identity = 'worker-%s-%s' % (name, i)
  worker.connect('ipc://%s-localbe.ipc' % name)
  worker.send("READY")

  while True:
    try:
      msg = worker.recv_multipart()
    except zmq.ZMQError:
      return
    print 'worker-%s: %s' % (i, msg)
    msg[-1] = 'OK'
    worker.send_multipart(msg)



def main(myself, peers):
  print 'I: preparing broker at %s ...' % myself
  ctx = zmq.Context()

  cloudfe = ctx.socket(zmq.ROUTER)
  cloudfe.setsockopt(zmq.IDENTITY, myself)
  cloudfe.bind('ipc://%s-cloud.ipc' % myself)

  cloudbe = ctx.socket(zmq.ROUTER)
  cloudbe.setsockopt(zmq.IDENTITY, myself)
  for peer in peers:
    print 'I: connecting to cloud frontend at %s' % peer 
    cloudbe.connect('ipc://%s-cloud.ipc' % peer)
  
  localfe = ctx.socket(zmq.ROUTER)
  localfe.bind('ipc://%s-localfe.ipc' % myself)
  localbe = ctx.socket(zmq.ROUTER)
  localbe.bind('ipc://%s-localbe.ipc' % myself)

  raw_input('Press Enter when all brokers are started :')

  for i in range(NUM_WORKERS):
    thread = threading.Thread(target=worker_routine, args=(myself, i))
    thread.daemon = True
    thread.start()
  for i in range(NUM_CLIENTS):
    thread = threading.Thread(target=client_routine, args=(myself, i))
    thread.daemon = True
    thread.start()

 
  workers = []
  pollbe = zmq.Poller()
  pollbe.register(localbe, zmq.POLLIN)
  pollbe.register(cloudbe, zmq.POLLIN)

  pollfe = zmq.Poller()
  pollfe.register(localfe, zmq.POLLIN)
  pollfe.register(cloudfe, zmq.POLLIN)

  while True:
    try:
      events = dict(pollbe.poll(1000 if workers else None))
    except zmq.ZMQError:
      break

    msg = None 
    if localbe in events:
      msg = localbe.recv_multipart()
      (address, empty), msg = msg[:2], msg[2:]
      workers.append(address)
      if msg[-1] == 'READY':
        msg = None 
    elif cloudbe in events:
      msg = cloudbe.recv_multipart()
      (address, empty), msg = msg[:2], msg[2:]
    
    if msg is not None:
      address = msg[0]
      if address in peers:
        cloudfe.send_multipart(msg)
      else:
        localbe.send_multipart(msg)
    
    while workers:
      events = dict(pollfe.poll(0))
      reroutable = False
      if cloudfe in events:
        msg = cloudfe.recv_multipart()
        reroutable = False 
      elif localfe in events:
        msg = localfe.recv_multipart()
        reroutable = True 
      else:
        break 
      
      if reroutable and peers and random.randint(0, 4) == 0:
        msg = [random.choice(peers), ''] + msg
        cloudbe.send_multipart(msg)
      else:
        msg = [workers.pop(0), ''] + msg 
        localbe.send_multipart(msg)



if __name__ == '__main__':
  if len(sys.argv) >= 2:
    main(myself=sys.argv[1], peers=sys.argv[2:])
  else:
    print 'Usage: peering.py <me> [<you_1> ... <you_n>]'
    exit(1)
