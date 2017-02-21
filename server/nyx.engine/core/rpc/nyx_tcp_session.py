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
import socket
from cStringIO import StringIO
from log.nyx_log import LogManager

# [doc]
# channel
#   - on_disconnected() # TODO: 需要实现channel的on_disconnected回调
#   - input_data(...) # TODO: 需要实现channel的input_data接口
#
# encrypter
#   - encrypt(...) # TODO: 需要有encrypt接口
#
# decrypter
#   - decrypt(...) # TODO: 需要有decrypt接口
#
# compressor
#   - compress(...) # TODO: 需要有加压缩接口
#   - decompress(...) # TODO: 需要有解压缩接口

class TcpSession(asyncore.dispatcher):
    """一条建立好的网络链接，可以是服务器的accept也可以是客户端的connect"""
    _DEF_RECV_BYTES = 4096 # 默认接收buffer大小
    _STATUS_INIT = 0
    _STATUS_ESTABLISHED = 1
    _STATUS_DISCONNECTED = 2

    def __init__(self, sock, peername):
        super(TcpSession, self).__init__(sock)

        self._logger = LogManager.get_logger('NyxCore.Rpc.TcpSession')
        self._status = TcpSession._STATUS_INIT
        self._writbuf = StringIO()
        if sock:
            self._status = TcpSession._STATUS_ESTABLISHED
        self._recvbuf_bytes = TcpSession._DEF_RECV_BYTES
        self._channel = None
        self._peername = peername

        self._encrypter = None
        self._decrypter = None
        self._compressor = None

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
        self._channel = channel

    def get_channel(self):
        return self._channel

    def set_compressor(self, compressor):
        self._compressor = compressor

    def set_crypter(self, encrypter, decrypter):
        self._encrypter = encrypter
        self._decrypter = decrypter

    def is_established(self):
        """判断连接是否已建立"""
        return self._status == TcpSession._STATUS_ESTABLISHED

    def set_recv_buffer(self, recv_bytes):
        """设置接收缓冲区的大小"""
        self._recvbuf_bytes = recv_bytes

    def disconnect(self, flush=True):
        """断开连接"""
        if self._status == TcpSession._STATUS_DISCONNECTED:
            return
        if self._channel:
            self._channel.on_disconnected()
            self._channel = None
        if self.socket:
            if self.writable() and flush:
                self.handle_write()
            super(TcpSession, self).close()
        self._status = TcpSession._STATUS_DISCONNECTED

    def get_peername(self):
        """获取对端(ip, port)"""
        return self._peername

    def handle_close(self):
        """断开连接的时候回调"""
        super(TcpSession, self).handle_close()
        self.disconnect(flush=False)

    def handle_expt(self):
        """连接异常的时候回调"""
        super(TcpSession, self).handle_expt()
        self.disconnect(flush=False)

    def handle_error(self):
        """连接出错的时候回调"""
        super(TcpSession, self).handle_error()
        self.disconnect(flush=False)

    def handle_read(self):
        """数据可读的时候回调"""
        data = self.recv(self._recvbuf_bytes)
        if data:
            if self._channel is None:
                return

            if self._decrypter:
                data = self._decrypter.decrypt(data)
            if self._compressor:
                data = self._compressor.decompress(data)
            r = self._channel.input_data(data)

            if r == 2:
                return
            elif r == 3:
                self._logger.error('TcpSession.handle_read - buffer length error')
            elif r == 0:
                self.disconnect(flush=False)
                return
            else:
                self._logger.warn('TcpSession.handle_read - return(%d), close socket with(%s)', r, self.get_peername())
                self.disconnect(flush=False)
                return

    def handle_write(self):
        """数据可写的时候回调"""
        buf = self._writbuf.getvalue()
        if buf:
            nwrote = self.send(buf)
            self._writbuf = StringIO(buf[nwrote:])
            self._writbuf.seek(0, 2)

    def write_data(self, data):
        """发送数据信息"""
        if self._compressor:
            data = self._compressor.compress(data)
        if self._encrypter:
            data = self._encrypter.encrypt(data)
        self._writbuf.write(data)

    def writable(self):
        return (self._writbuf and self._writbuf.getvalue()) or (self._status != TcpSession._STATUS_ESTABLISHED)
