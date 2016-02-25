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
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO
from MarsLog.LogManager import LogManager
from Utils.Const import *

class TcpConnector(asyncore.dispatcher):
    """一条建立好的连接

    来自TcpServer的accept或TcpClient的connect,
    channelObj处理断开的连接和数据的读写
    """
    DEF_RECV_BUFFER = 4096
    ST_INIT = 0
    ST_ESTABLISHED = 1
    ST_DISCONNECTED = 2

    def __init__(self, fd, peername):
        super(TcpConnector, self).__init__(fd)

        self.status = TcpConnector.ST_INIT
        self.wBuffer = StringIO()

        if fd:
            self.status = TcpConnector.ST_ESTABLISHED

        self.logger = LogManager.getLogger('MarsRpc.TcpConnector')
        self.recvBuffSize = TcpConnector.DEF_RECV_BUFFER
        self.channelObj = None
        self.peername = peername

        fd and self.setOption()

    def setOption(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        if hasattr(socket, 'TCP_KEEPCNT') and hasattr(socket, 'TCP_KEEPIDLE') and hasattr(socket, 'TCP_KEEPINTVL'):
            self.socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3)
            self.logger.debug('setOption TCP_KEEPCNT')
            self.socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 60)
            self.logger.debug('setOption TCP_KEEPIDLE')
            self.socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 60)
            self.logger.debug('setOption TCP_KEEPINTVL')

    def setChannelObj(self, channelObj):
        """设置channel object, 用于接收数据"""
        self.channelObj = channelObj
        self.logger.debug('setChannelObj')

    def getChannelObj(self):
        return self.channelObj

    def established(self):
        """判断是否建立连接"""
        return self.status == TcpConnector.ST_ESTABLISHED

    def setRecvBuffer(self, size):
        """设置接收buffer的大小"""
        self.recvBuffSize = size

    def disconnect(self, flush=True):
        """断开连接"""
        if self.status == TcpConnector.ST_DISCONNECTED:
            return

        if self.channelObj:
            self.channelObj.onDisconnected()
        self.channelObj = None
        if self.socket:
            if self.writable() and flush:
                self.handle_write()
            super(TcpConnector, self).close()
        self.status = TcpConnector.ST_DISCONNECTED
        self.logger.info('disconnect with %s', self.getPeername())

    def getPeername(self):
        """获取对端信息(ip, port)"""
        return self.peername

    def handle_close(self):
        """连接断开回调"""
        super(TcpConnector, self).handle_close()
        self.disconnect(False)

    def handle_expt(self):
        """连接异常回调"""
        super(TcpConnector, self).handle_expt()
        self.disconnect(False)

    def handle_error(self):
        """连接出错回调"""
        super(TcpConnector, self).handle_error()
        self.disconnect(False)

    def handle_read(self):
        """读取数据回调"""
        data = self.recv(self.recvBuffSize)
        if data:
            if self.channelObj is None:
                return

            rc = self.channelObj.onRead(data)
            if rc == MARS_RC_SUCCESSED:
                return
            elif rc == MARS_RC_FAILED:
                self.disconnect(False)
                return
            else:
                self.logger.warn('handle_read return %d, close socket with %s', rc, self.getPeername())
                self.disconnect(False)

    def handle_write(self):
        """发送数据回调"""
        buff = self.wBuffer.getvalue()
        if buff:
            sentBytes = self.send(buff)
            self.wBuffer = StringIO(buff[sentBytes:])
            self.wBuffer.seek(0, 2)

    def writeData(self, data):
        """发送数据"""
        self.wBuffer.write(data)

    def writable(self):
        return (self.wBuffer and self.wBuffer.getvalue()) or (self.status != TcpConnector.ST_ESTABLISHED)
