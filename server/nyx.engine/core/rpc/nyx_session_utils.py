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

from log.nyx_log import LogManager

class SessionWithCryptCompress(object):
    """用于封装加密、压缩后的session"""
    def __init__(self, conn, encrypter=None, decrypter=None, compressor=None):
        self._logger = LogManager.get_logger('NyxCore.Rpc.SessionWithCryptCompress')
        self._conn = conn
        self._channel = conn.get_channel()
        conn.set_channel(self)
        self._encrypter = encrypter
        self._decrypter = decrypter
        self._compressor = compressor

    def set_channel(self, channel):
        self._channel = channel

    def set_compressor(self, compressor):
        self._compressor = compressor

    def set_crypter(self, encrypter, decrypter):
        self._encrypter = encrypter
        self._decrypter = decrypter

    def get_crypter(self):
        return (self._encrypter, self._decrypter)

    def set_recv_buffer(self, recv_bytes):
        self._conn.set_recv_buffer(recv_bytes)

    def is_established(self):
        return self._conn.is_established()

    def get_peeraddr(self):
        return self._conn.get_peeraddr()

    def disconnect(self):
        self._conn.disconnect()

    def input_data(self, data):
        """解密，解压数据信息"""
        if self._decrypter:
            data = self._decrypter.decrypt(data)
        if self._compressor:
            data = self._compressor.decompress(data)
        return self._channel.input_data(data)

    def on_disconnected(self):
        """连接断开的回调"""
        if self._channel:
            self._channel.on_disconnected()
        self._encrypter = None
        self._decrypter = None
        self._compressor = None

    def write_data(self, data):
        """压缩、加密数据以便发送"""
        if self._compressor:
            data = self._compressor.compress(data)
        if self._encrypter:
            data = self._encrypter.encrypt(data)
        self._conn.write_data(data)
