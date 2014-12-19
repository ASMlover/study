#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
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

import sys
import socket
import signal
import threading

ADDRESS = ('', 5555)
RECVBUF = 1024

class TServer(object):
    def __init__(self):
        self.listenfd = None
        self.running = False

    def init(self):
        self.listenfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.listenfd.bind(ADDRESS)
        self.listenfd.listen(socket.SOMAXCONN)
        self.running = True

    def stop(self, signum, frame):
        self.listenfd.close()
        self.running = False

    def message_handler(self):
        print 'start server success ...'
        while self.running:
            clientfd, addr = self.listenfd.accept()
            if clientfd is not None:
                recvdata = clientfd.recv(RECVBUF)
                if recvdata is not None:
                    print 'recevice data from client', addr, recvdata
                    clientfd.send(recvdata)
                clientfd.close()

    def run(self):
        signal.signal(signal.SIGINT, self.stop)
        signal.signal(signal.SIGTERM, self.stop)

        trd = threading.Thread(target=self.message_handler)
        trd.start()

        while True:
            try:
                trd.join()
            except KeyboardInterrupt:
                sys.exit(0)

if __name__ == '__main__':
    server = TServer()
    server.init()
    server.run()
