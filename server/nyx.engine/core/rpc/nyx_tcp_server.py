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
from log.nyx_log import LogManager
from rpc.nyx_tcp_session import TcpSession

# [doc]
# connection_handler
#   - on_new_connection(...) # TODO: 需要实现on_new_connection回调

class TcpServer(asyncore.dispatcher):
    """负责监听TCP连接，创建连接"""
    def __init__(self, addr, port, conn_handler=None, reuse_addr=False):
        super(TcpServer, self).__init__()

        self._logger = LogManager.get_logger('NyxCore.Rpc.TcpServer')
        self._addr = addr
        self._port = port
        self._conn_handler = conn_handler
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        if reuse_addr:
            self.set_reuse_addr()
        self._started = False
        self._try_bind()
        self.listen(50)

    def _try_bind(self):
        while True:
            try:
                self.bind((self._addr, self._port))
                break
            except:
                self._logger.warn('TcpServer._try_bind - failed to bind(%s:%s)', self._addr, self._port)
                self._port += 1
                if self._port > 65535:
                    self._logger.error('TcpServer._try_bind - failed to find a usable port(%d)', self._port)
                    raise StandardError('failed to find a usable port')
        self._started = True

    def get_localaddr(self):
        return (self._addr, self._port)

    def set_connection_handler(self, conn_handler):
        self._conn_handler = conn_handler

    def stop(self):
        self.close()
        self._started = False

    def close(self):
        super(TcpServer, self).close()

    def handle_accept(self):
        """连接事件回调"""
        try:
            sock, addr = self.accept()
        except socket.error:
            self._logger.warn('TcpServer.handle_accept - throw an exception')
            self._logger.nyxlog_exception()
            return
        except TypeError:
            self._logger.warn('TcpServer.handle_accept - throw EWOULDBLOCK')
            self._logger.nyxlog_exception()
            return

        if self._conn_handler:
            conn = TcpSession(sock, addr)
            self._conn_handler.on_new_connection(conn)
        else:
            self._logger.warn('TcpServer.handle_accept - no connection to handled')

    def handle_error(self):
        """出错的时候回调"""
        self._logger.error('TcpServer.handle_error - error: %s', str(inspect.stack()))

    def handle_close(self):
        """关闭的时候回调"""
        self._logger.info('TcpServer.handle_close - called from: %s', str(inspect.stack()))
