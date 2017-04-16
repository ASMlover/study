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
import inspect
import socket
from Log.LogManager import LogManager
from Rpc.TcpSession import TcpSession

class TcpServer(asyncore.dispatcher):
    """负责监听TCP连接，创建客户连接，可以创建多个TCP连接

    依赖connector handler，需要实现处理新连接的回调：
        on_new_connection - 处理新连接
    """
    def __init__(self, ip, port, conn_handler=None, reuse_addr=False):
        asyncore.dispatcher.__init__(self)
        self.logger = LogManager.get_logger('Rpc.TcpServer')

        self.ip = ip
        self.port = port
        self.conn_handler = conn_handler
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        if reuse_addr:
            self.set_reuse_addr()
        self.started = False
        self.try_bind()
        self.listen(50)

    def try_bind(self):
        while True:
            try:
                self.bind((self.ip, self.port))
                break
            except:
                self.logger.warn('TcpServer.try_bind - bind(%s: %s) failed',
                        self.ip, self.port)
                self.port += 1
                if self.port > 65535:
                    self.logger.error(
                            'TcpServer.try_bind - find usable port(%s) failed',
                            self.port)
                    raise StandardError('failed to find a usable port')
        self.started = True

    def get_localaddr(self):
        return (self.ip, self.port)

    def set_connection_handler(self, conn_handler):
        self.conn_handler = conn_handler

    def stop(self):
        self.close()
        self.started = False

    def close(self):
        super(TcpServer, self).close()

    def handle_accept(self):
        """有客户端连接的时候回调"""
        try:
            sock, addr = self.accept()
        except socket.error:
            self.logger.warn('TcpServer.handle_accept - throw an exception')
            self._log_exception()
            return
        except TypeError:
            self.logger.warn('TcpServer.handle_accept - throw EWOULDBLOCK')
            self._log_exception()
            return

        if self.conn_handler:
            conn = TcpSession(sock, addr)
            self.conn_handler.on_new_connection(conn)
        else:
            self.logger.warn('TcpServer.handle_accept - no connection handle')

    def handle_close(self):
        """连接关闭的时候回调"""
        self.logger.info(
                'TcpServer.handle_close - call from(%s)', inspect.stack())

    def handle_error(self):
        """连接出错的时候回调"""
        self.logger.error(
                'TcpServer.handle_error - error(%s)', inspect.stack())
