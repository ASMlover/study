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

import socket
import time
import threading

ADDRESS = ('', 5555)
RECVBUF = 1024

def ServerMain():
    listenfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listenfd.bind(ADDRESS)
    listenfd.listen(socket.SOMAXCONN)
    running = True
    
    def MessageHandler():
        print 'start server suceess ...'
        while running:
            client, addr = listenfd.accept()
            if client is not None:
                data = client.recv(RECVBUF)
                if data is not None:
                    print 'recevice from client', addr, data
                    client.send(data)
                client.close()

    trd = threading.Thread(target=MessageHandler)
    trd.start()

    try:
        while running:
            time.sleep(1)
    except KeyboardInterrupt:
        running = False
        listenfd.close()

if __name__ == '__main__':
    ServerMain()
