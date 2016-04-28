#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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
import const
from pyRpc.logger import LoggerMgr

class TcpConnection(asyncore.dispatcher):
    ST_INIT         = 0
    ST_ESTABLISHED  = 1
    ST_DISCONNECTED = 2

    def __init__(self, fd, peername):
        super(TcpConnection, self).__init__(fd)
        self.logger = LoggerMgr.getLogger('pyRpc.TcpConnection')
        self.peername = peername

        self.writebuf = ''
        self.readbuf_len = const.RPC_READBUF_LEN
        self.status = TcpConnection.ST_INIT

        if fd:
            self.status = TcpConnection.ST_DISCONNECTED
            self.set_socket_option()
        self.rpc_channel = None

    def set_socket_option(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)

    def set_rpc_channel(self, rpc_channel):
        self.rpc_channel = rpc_channel

    def get_rpc_channel(self):
        return self.rpc_channel

    def is_established(self):
        return self.status == TcpConnection.ST_ESTABLISHED

    def set_readbuf_len(self, readbuf_len):
        self.readbuf_len = readbuf_len

    def get_peername(self):
        return self.peername

    def writable(self):
        return len(self.writebuf) > 0

    def write_data(self, data):
        self.writebuf += data

    def disconnect(self):
        if self.status == TcpConnection.ST_DISCONNECTED:
            return
        if self.rpc_channel:
            self.rpc_channel.on_disconnected()
            self.rpc_channel = None

        if self.socket:
            super(TcpConnection, self).close()
        self.status = TcpConnection.ST_DISCONNECTED

    def handle_read(self):
        self.logger.debug('TcpConnection.handle_read')
        data = self.recv(self.readbuf_len)
        if data:
            if not self.rpc_channel:
                return
            self.rpc_channel.on_read(data)

    def handle_write(self):
        self.logger.debug('TcpConnection.handle_write')
        if self.writebuf:
            size = self.send(self.writebuf)
            self.writebuf = self.writebuf[size:]

    def handle_close(self):
        self.logger.debug('TcpConnection.handle_close')
        super(TcpConnection, self).handle_close()
        self.disconnect()

    def handle_error(self):
        self.logger.debug('TcpConnection.handle_error')
        super(TcpConnection, self).handle_error()
        self.disconnect()

    def handle_expt(self):
        self.logger.debug('TcpConnection.handle_expt')
        super(TcpConnection, self).handle_expt()
        self.disconnect()
