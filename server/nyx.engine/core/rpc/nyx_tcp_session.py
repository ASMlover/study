#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2017 ASMlover. All rights reserved.
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

import asyncore
import socket
from StringIO import StringIO
from log.nyx_log import LogManager

class TcpSession(asyncore.dispatcher):
    """一条建立好的网络链接，可以是服务器的accept也可以是客户端的connect"""
    _DEF_RECV_BYTES = 4096 # 默认接收buffer大小
    _STATUS_INIT = 0
    _STATUS_ESTABLISHED = 1
    _STATUS_DISCONNECTED = 2

    def __init__(self, sock, peername):
        super(TcpSession, self).__init__(sock)

        self._logger = LogManager.get_logger('NyxCore.Rpc.TcpSession')
        self._status = TcpSession._STATUS_INIT
        self._writbuf = StringIO()
        if sock:
            self._status = TcpSession._STATUS_ESTABLISHED
        self._recvbuf_bytes = TcpSession._DEF_RECV_BYTES
        self._channel = None
        self._peername = peername

        self._encrypter = None
        self._decrypter = None
        self._compressor = None

        if sock:
            self.set_sockoption()

    def set_sockoption(self):
        pass

    def set_channel(self, channel):
        self._channel = channel

    def get_channel(self):
        return self._channel

    def set_compressor(self, compressor):
        self._compressor = compressor

    def set_crypter(self, encrypter, decrypter):
        self._encrypter = encrypter
        self._decrypter = decrypter

    def is_established(self):
        return self._status == TcpSession._STATUS_ESTABLISHED

    def set_recv_buffer(self, recv_bytes):
        self._recvbuf_bytes = recv_bytes

    def disconnect(self, flush=True):
        pass

    def get_peername(self):
        return self._peername

    def handle_close(self):
        pass

    def handle_expt(self):
        pass

    def handle_read(self):
        pass

    def handle_write(self):
        pass

    def write_data(self, data):
        pass

    def writable(self):
        return (self._writbuf and self._writbuf.getvalue()) or (self._status != TcpSession._STATUS_ESTABLISHED)
