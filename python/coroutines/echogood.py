#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2020 ASMlover. All rights reserved.
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
from pyos7 import *
from typing import Any, Generator


SockGenerator = Generator[Any, Any, Any]

def handle_client(client: socket.socket, addr: tuple) -> SockGenerator:
    print(f"connection from: {addr}")
    while True:
        yield ReadWait(client)
        data = client.recv(65536)
        if not data:
            break
        yield WriteWait(client)
        client.send(data)
    client.close()
    print(f"client closed")

def server(port: int) -> SockGenerator:
    print(f"server starting ...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("", port))
    sock.listen(5)
    while True:
        yield ReadWait(sock)
        client, addr = sock.accept()
        yield NewTask(handle_client(client, addr))

def alive() -> SockGenerator:
    while True:
        print(f"I'm alive")
        yield


if __name__ == '__main__':
    sched = Scheduler()
    sched.new(alive())
    sched.new(server(5555))
    sched.mainloop()
