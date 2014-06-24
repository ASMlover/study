# Copyright (c) 2014 ASMlover. All rights reserved.
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
#
#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import socket
import select


def start_server():
  s = socket.socket()
  host = socket.gethostname()
  port = 1234
  s.bind((host, port))

  fdmap = {s.fileno(): s}

  s.listen(5)
  poll = select.poll()
  poll.register(s)
  while True:
    events = poll.poll()
    for fd, ev in events:
      if fd in fdmap:
        c, addr = s.accept()
        print 'Got connection from', addr 
        poll.register(c)
        fdmap[c.fileno()] = c
      elif ev & select.POLLIN:
        data = fdmap[fd].recv(1024)
        if not data:
          print fdmap[fd].getpeername(), 'disconnected'
          poll.unregister(fd)
          del fdmap[fd]
        else:
          print data 



if __name__ == '__main__':
  start_server()
