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
from cStringIO import StringIO
import socket
from Log.LogManager import LogManager

class TcpSession(asyncore.dispatcher):
    """建立好的网络链接

    可以是服务器的accept connector也可以是客户端的connector
    """
    _DEFAULT_RECV_BYTES = 4096 # 默认接收buffer的大小
    _STATUS_INIT = 0
    _STATUS_ESTABLISHED = 1
    _STATUS_DISCONNECTED = 2

    def __init__(self, sock, peeraddr):
        super(TcpSession, self).__init__(sock)
        self.logger = LogManager.get_logger('Rpc.TcpSession')

        self.status = TcpSession._STATUS_INIT
        self.writbuf = StringIO() # socket session的write buffer
        if sock:
            self.status = TcpSession._STATUS_ESTABLISHED
        self.recvbuf_bytes = TcpSession._DEFAULT_RECV_BYTES
        self.channel = None
        self.peeraddr = peeraddr

        self.encrypter = None
        self.decrypter = None
        self.compressor = None

        if sock:
            self.set_sockoption()

    def set_sockoption(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        if (hasattr(socket, 'TCP_KEEPCNT')
                and hasattr(socket, 'TCP_KEEPIDLE')
                and hasattr(socket, 'TCP_KEEPINTVL')):
            self.socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3)
            self.socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 60)
            self.socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 60)

    def set_channel(self, channel):
        self.channel = channel

    def get_channel(self):
        return self.channel

    def set_crypter(self, encrypter, decrypter):
        self.encrypter = encrypter
        self.decrypter = decrypter

    def set_compressor(self, compressor):
        self.compressor = compressor

    def is_established(self):
        """判断连接是否已经建立"""
        return self.status == TcpSession._STATUS_ESTABLISHED

    def set_recv_buffer(self, recv_bytes):
        """设置接收缓冲区的大小"""
        self.recvbuf_bytes = recv_bytes

    def get_peeraddr(self):
        """获取连接对端的(ip地址, port端口)"""
        return self.peeraddr

    def disconnect(self, flush=True):
        """断开连接"""
        if self.status == TcpSession._STATUS_DISCONNECTED:
            return

        if self.channel:
            self.channel.on_disconnected() # 调用channel的on_disconnected
            self.channel = None
        if self.socket:
            if self.writable() and flush:
                self.handle_write()
            super(TcpSession, self).close()
        self.status = TcpSession._STATUS_DISCONNECTED

    def write_data(self, data):
        """向网络对端发送数据信息"""
        if self.compressor:
            data = self.compressor.compress(data)
        if self.encrypter:
            data = self.encrypter.encryt(data)
        self.writbuf.write(data)

    def writable(self):
        # 重写基类writable，判断是否数据可写
        return ((self.writbuf and self.writbuf.getvalue())
                or (self.status != TcpSession._STATUS_ESTABLISHED))

    # override基类的事件处理handler
    def handle_close(self):
        """断开连接时回调"""
        super(TcpSession, self).handle_close()
        self.disconnect(flush=False)

    def handle_expt(self):
        """连接异常时回调"""
        super(TcpSession, self).handle_expt()
        self.disconnect(flush=False)

    def handle_error(self):
        """连接出错或读写出错时回调"""
        super(TcpSession, self).handle_error()
        self.disconnect(flush=False)

    def handle_read(self):
        """连接有数据可读时回调"""
        data = self.recv(self.recvbuf_bytes)
        if data:
            if self.channel is None:
                return

            if self.decrypter:
                data = self.decrypter.decrypt(data)
            if self.compressor:
                data = self.compressor.decompress(data)
            r = self.channel.on_input_data(data) # 调用channel的on_input_data

            if r == 0:
                self.disconnect(flush=False)
            elif r == 2:
                return
            elif r == 3:
                self.logger.error('TcpSession.handle_read - length error')
            else:
                self.logger.warn('TcpSession.handle_read - return(%d)', r)
                self.disconnect(flush=False)

    def handle_write(self):
        """连接数据可写时回调"""
        buf = self.writbuf.getvalue()
        if buf:
            nwrote = self.send(buf)
            self.writbuf = StringIO(buf[nwrote:])
            self.writbuf.seek(0, 2)
