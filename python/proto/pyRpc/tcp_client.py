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

import socket
from pyRpc.logger import LoggerMgr
from pyRpc.rpc_channel import RpcChannel
from pyRpc.tcp_connection import TcpConnection

class TcpClient(TcpConnection):
    def __init__(self, host, port, service_creator, stub_creator):
        super(TcpClient, self).__init__(None, (host, port))
        self.logger = LoggerMgr.get_logger('pyRpc.TcpClient')
        self.service_creator = service_creator
        self.stub_creator = stub_creator
        self.conn_handler = None
        self.channel = None
        self.service = None
        self.stub = None

    def close(self):
        self.disconnect()

    def set_connection_handler(self, conn_handler):
        self.conn_handler = conn_handler

    def sync_connect(self):
        fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            fd.connect(self.peername)
        except socket.error as e:
            self.logger.warn('TcpClient.sync_connect: connect to %s failed: %s', self.peername, e)
            fd.close()
            return False

        fd.setblocking(0)
        self.set_socket(fd)
        self.set_socket_option()

        self.status = TcpConnection.ST_ESTABLISHED
        self.conn_handler.on_new_connection(self)
        return True

    def async_connect(self):
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_socket_option()
        self.connect(self.peername)

    def writable(self):
        if self.status == TcpConnection.ST_ESTABLISHED:
            return super(TcpClient, self).writable()
        else:
            return True

    def handle_connect(self):
        self.logger.debug('TcpClient.handle_connect: connection established')
        self.status = TcpConnection.ST_ESTABLISHED

        self.service = self.service_creator()
        self.channel = RpcChannel(self.service, self)
        self.stub = self.stub_creator(self.channel)

    def handle_close(self):
        self.logger.debug('TcpClient.handle_close: connection closed')
        super(TcpClient, self).handle_close()
