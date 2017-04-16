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

import socket
from Log.LogManager import LogManager
from Rpc.TcpSession import TcpSession

class TcpClient(TcpSession):
    """Tcp客户端，建立连接

    依赖connector handler的实现，需要实现:
        on_new_connection - 处理连接成功回调
        on_connection_closed - 处理连接关闭回调
    """
    def __init__(self, ip, port, conn_handler=None):
        super(TcpClient, self).__init__(None, (ip, port))
        self.logger = LogManager.get_logger('Rpc.TcpClient')

        self.conn_handler = conn_handler

    def set_connection_handler(self, conn_handler):
        self.conn_handler = conn_handler

    def close(self):
        self.disconnect(flush=False)

    def sync_connect(self):
        """同步连接"""
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            sock.connect(self.peeraddr)
        except socket.error as e:
            sock.close()
            self.logger.warn('TcpClient.sync_connect - connect failed(%s)', e)
            return False

        sock.setblocking(False)
        self.set_socket(sock)
        self.set_sockoption()
        self.status = TcpSession._STATUS_ESTABLISHED
        return True

    def async_connect(self):
        """异步连接"""
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_sockoption()
        self.connect(self.peeraddr)

    def handle_connect(self):
        """连接成功时回调"""
        if self.conn_handler:
            self.status = TcpSession._STATUS_ESTABLISHED
            self.conn_handler.on_new_connection(self)
        else:
            self.logger.warn('TcpClient.handle_connect - no new connection')

    def handle_close(self):
        """连接关闭时回调"""
        super(TcpClient, self).handle_close()
        self.conn_handler.on_connection_closed(self)
