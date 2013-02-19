#!/usr/bin/python
# -*- coding: utf-8 -*-

import zmq
import time


if __name__ == '__main__':
  ctx = zmq.Context()
  s = ctx.socket(zmq.REQ)
  s.connect('tcp://localhost:5555')

  for i in range(10):
    s.send('Hello, world! count [%d]' % i)
    msg = s.recv()
    print 'reply message is : %s' % msg 

    time.sleep(0.1)
  s.close()
