#!/usr/bin/python
# -*- coding: utf-8 -*-

import zmq


if __name__ == '__main__':
  ctx = zmq.Context()
  s = ctx.socket(zmq.REP)
  s.bind('tcp://*:5555')

  print 'replay serevr init success ...'
  while True:
    try:
      msg = s.recv()
      s.send(msg)
    except KeyboardInterrupt:
      break
  s.close()
