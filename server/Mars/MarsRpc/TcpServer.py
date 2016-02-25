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

import PathHelper as PH
PH.addPathes('../')

import asyncore
import inspect
import socket
import sys
import time

import AsyncIo as asio
from ChannelObjs import EchoChannelObj
from ConnectorManager import ConnectorManager
from MarsLog.LogManager import LogManager
from TcpConnector import TcpConnector

class TcpServer(asyncore.dispatcher):
    """负责监听TCP连接

    建立连接后交由上层的connector handler来处理，connector handler
    需要注册一个handlerNewConnector函数来处理新连接
    """

    def __init__(self, ip, port, connectorHandler=None, reuseAddr=False):
        super(TcpServer, self).__init__()

        self.logger = LogManager.getLogger('MarsRpc.TcpServer')
        self.ip = ip
        self.port = port
        self.connectorHandler = connectorHandler
        self.create_socket(socket.AF_INT, socket.SOCK_STREAM)
        if reuseAddr:
            self.set_reuse_addr()
        self.started = False
        self.tryBind()
        self.logger.info('TcpServer init: server liston on: %s, %d', self.ip, self.port)
        self.listen(50)

    def tryBind(self):
        pass

    def stop(self):
        self.close()
        self.started = False
        self.logger.info('stop')

    def close(self):
        super(TcpServer, self).close()
        self.logger.info('close: called from: %s', str(inspect.stack()))

    def getListen(self):
        return (self.ip, self.port)

    def setConnectorHandler(self, connectorHandler):
        self.connectorHandler = connectorHandler

    def handle_accept(self):
        """连接回调"""
        pass

    def handle_error(self):
        """错误回调"""
        pass

    def handle_close(self):
        """关闭连接回调"""
        pass
