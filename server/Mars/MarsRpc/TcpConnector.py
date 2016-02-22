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
import socket
import StringIO import StringIO
from MarsLog.LogManager import LogManager

class TcpConnector(asyncore.dispatcher):
    """一条建立好的连接

    来自TcpServer的accept或TcpClient的connect,
    channelObj处理断开的连接和数据的读写
    """
    DEF_RECV_BUFFER = 4096
    ST_INIT = 0
    ST_ESTABLISHED = 1
    ST_DISCONNECTED = 2

    def __init__(self, sock, peerName):
        super(TcpConnector, self).__init__(sock)

        self.status = TcpConnector.ST_INIT
        self.wBuffer = StringIO()

        if sock:
            self.status = TcpConnector.ST_ESTABLISHED

        self.logger = LogManager.getLogger('MarsRpc.TcpConnector')
        self.recvBuffSize = TcpConnector.DEF_RECV_BUFFER
        self.channelObj = None
        self.peerName = peerName

        sock and self.setOption()

    def setOption(self):
        pass

    def setChannelObj(self, channelObj):
        pass

    def getChannelObj(self):
        pass

    def established(self):
        pass

    def setRecvBuffer(self, size):
        pass

    def disconnect(self, flush=True):
        pass

    def getPeerName(self):
        return

    def handle_close(self):
        """连接断开回调"""
        pass

    def handle_expt(self):
        """连接异常回调"""
        pass

    def handle_error(self):
        """连接出错回调"""
        pass

    def handle_read(self):
        """读取数据回调"""
        pass

    def handle_write(self):
        """发送数据回调"""
        pass

    def sendData(self, data):
        pass

    def writable(self):
        pass
