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

import socket
import sys
import time

import AsyncIo as asio
from ChannelObjs import LoggingChannelObj
from ConnectorManager import ConnectorManager
from MarsLog.LogManager import LogManager
from TcpConnector import TcpConnector

class TcpClient(TcpConnector):
    """TCP客户端"""

    def __init__(self, ip, port, connectorHandler=None):
        super(TcpClient, self).__init__(None, (ip, port))
        self.logger = LogManager.getLogger('MarsRpc.TcpClient')
        self.logger.info('init tcp client, connect to : %s', self.peerName)

    def close(self):
        """断开连接"""
        self.disconnect()

    def setConnectorHandler(self, connectorHandler):
        self.connectorHandler = connectorHandler

    def asyncConnect(self):
        self.create_socket(socket.AF_INT, socket.SOCK_STREAM)
        self.setOption()
        self.connect(self.peerName)

    def syncConnect(self):
        fd = socket.socket(socket.AF_INT, socket.SOCK_STREAM)
        try:
            fd.connect(self.peerName)
        except socket.error as err:
            fd.close()
            self.logger.warn('syncConnect failed %s with remote server %s', err, self.peerName)
            return False
        fd.setblocking(0)
        self.set_socket(fd)
        self.setOption()
        self.status = TcpConnector.ST_ESTABLISHED
        return True

    def handle_connect(self):
        """建立连接时回调"""
        if self.connectorHandler:
            self.status = TcpConnector.ST_ESTABLISHED
            self.connectorHandler.handleNewConnector(self)
        else:
            self.logger.warn('no connector manager to handler new connection')

    def handle_close(self):
        """断开连接时回调"""
        super(TcpClient, self).handle_close()
        self.connectorHandler.handleConnectorFailed(self)
