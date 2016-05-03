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

from pyRpc.logger import LoggerMgr
from pyRpc.rpc_channel import RpcChannel
from pyRpc.tcp_connection import TcpConnection

class TcpServer(asyncore.dispatcher):
    def __init__(self, host, port, service_creator):
        super(TcpServer, self).__init__()
        self.logger = LoggerMgr.get_logger('pyRpc.TcpServer')
        self.host = host
        self.port = port
        self.service_creator = service_creator

        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((self.host, self.port))
        self.listen(50)
        self.logger.info('TcpServer.__init__: server listening on %s', str((self.host, self.port)))

    def stop(self):
        self.close()

    def on_new_connection(self, conn):
        self.logger.info('TcpServer.on_new_connection')
        service = self.service_creator()
        RpcChannel(service, conn)

    def handle_accept(self):
        try:
            fd, addr = self.accept()
        except socket.error as e:
            self.logger.error('TcpServer.handle_accept: accept error: %s', e.message)
            return
        except TypeError as e:
            self.logger.error('TcpServer.handle_accept: accept error: %s', e.message)
            return

        self.logger.debug('TcpServer.handle_accept: accept client from %s', addr)
        conn = TcpConnection(fd, addr)
        self.on_new_connection(conn)
