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
from dispatch import *

def _sock_accept(sock: socket.socket) -> DispGenerator:
    yield syscall("readwait", sock)
    yield sock.accept()

def _sock_write(sock: socket.socket, buf: bytes) -> DispGenerator:
    while buf:
        yield syscall("writwait", sock)
        nwrote = sock.send(buf)
        buf = buf[nwrote:]

def _sock_read(sock: socket.socket, max_bytes: int) -> DispGenerator:
    yield syscall("readwait", sock)
    yield sock.recv(max_bytes)


class Socket(object):
    def __init__(self, sock: socket.socket) -> None:
        self.sock = sock

    def accept(self) -> DispGenerator:
        yield syscall("readwait", self.sock)
        client, addr = self.sock.accept
        yield Socket(client), addr

    def write(self, buf: bytes) -> DispGenerator:
        _sock_write(self.sock)

    def read(self, max_bytes: int) -> DispGenerator:
        _sock_read(self.sock, max_bytes)

    def close(self):
        yield self.sock.close()
