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
import binascii


def dump(msg_or_socket):
  if isinstance(msg_or_socket, zmq.Socket):
    return dump(msg_or_socket.recv_multipart())
  else:
    msg = msg_or_socket

  print '============================================'
  for part in msg:
    print '[%03d] ' % len(part),
    is_text = True
    for c in part:
      if ord(c) < 32 or ord(c) > 127:
        is_text = False
        break
    if is_text:
      print part
    else:
      print binascii.hexlify(part)



if __name__ == '__main__':
  ctx = zmq.Context()

  sink = ctx.socket(zmq.ROUTER)
  sink.bind('inproc://example')

  no_id = ctx.socket(zmq.REQ)
  no_id.connect('inproc://example');
  no_id.send('ROUTER use a random generated UUID')
  dump(sink)
  no_id.close()

  with_id = ctx.socket(zmq.REQ)
  with_id.setsockopt(zmq.IDENTITY, 'with_id')
  with_id.connect('inproc://example')
  with_id.send('ROUTER use a REQ socket identity')
  dump(sink)
  with_id.close()
