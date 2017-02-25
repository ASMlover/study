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
from log.nyx_log import LogManager
from rpc.nyx_tcp_session import TcpSession

class TcpClient(TcpSession):
    """TCP客户端连接"""
    def __init__(self, addr, port, conn_handler=None):
        super(TcpClient, self).__init__(None, (addr, port))

        self._logger = LogManager.get_logger('NyxCore.Rpc.TcpClient')
        self._conn_handler = conn_handler

    def set_connection_handler(self, conn_handler):
        self._conn_handler = conn_handler

    def close(self):
        self.disconnect(flush=False)

    def async_connect(self):
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_sockoption()
        self.connect(self._peername)

    def sync_connect(self):
        pass

    def handle_connect(self):
        """连接成功的时候回调"""
        pass

    def handle_close(self):
        """连接关闭的时候回调"""
        pass
