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

from MarsLog.LogManager import LogManager

class CryptCompressConnector(object):
    """用于装饰Connector

    对Connector加入加密以及压缩
    """

    def __init__(self, connector, encrypter=None, decrypter=None, compressor=None):
        # 需要处理onRead和onDisconnected回调
        self.connector = connector
        self.channelObj = connector.getChannelObj()
        connector.setChannelObj(self)
        self.encrypter = encrypter
        self.decrypter = decrypter
        self.compressor = compressor
        self.logger = LogManager.getLogger('MarsRpc.CryptCompressConnector')

    def setChannelObj(self, channelObj):
        self.channelObj = channelObj

    def setCrypter(self, encrypter, decrypter):
        self.encrypter = encrypter
        self.decrypter = decrypter

    def getCrypter(self):
        return (self.encrypter, self.decrypter)

    def setCompressor(self, compressor):
        self. compressor = compressor

    def getCompressor(self):
        return self.compressor

    def setRecvBuffer(self, bufSize):
        self.connector.setRecvBuffer(bufSize)

    def established(self):
        return self.connector.established()

    def disconnect(self):
        self.connector.disconnect()

    def getPeername(self):
        return self.connector.getPeername()

    def writeData(self, data):
        if self.compressor:
            data = self.compressor.compress(data)
        if self.encrypter:
            data = self.encrypter.encrypt(data)
        self.connector.writeData(data)

    def onRead(self, data):
        if self.decrypter:
            data = self.decrypter.decrypt(data)
        if self.compressor:
            data = self.compressor.decompress(data)
        return self.channelObj.onRead(data)

    def onDisconnected(self):
        if self.channelObj:
            self.channelObj.onDisconnected()

        self.encrypter = None
        self.decrypter = None
        self.compressor = None
