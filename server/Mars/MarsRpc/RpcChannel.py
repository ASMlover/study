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

from struct import pack, unpack
from google.protobuf import service

import MarsRequest
from MarsRpc.ConnectorDecorator import CryptCompressConnector
from MarsRpc.LogManager import LogManager

class ProtobufLengthError(StandardError):
    pass

class RpcController(service.RpcController):
    """为上层提供RpcChannel的对象，protobuf rpc使用"""
    def __init__(self, channel):
        super(RpcController, self).__init__()
        self.rpcChannel = channel

class RpcChannel(service.RpcChannel):
    """序列化与反序列化rpc调用，内部封装一个底层连接(TcpConnector)"""

    def __init__(self, rpcService, connector):
        super(RpcChannel, self).__init__()

        self.rpcService = rpcService # 将rpc请求传递给上层
        self.rpcRequest = MarsRequest.Request() # rpc请求的解析
        self.rpcRequestParser = MarsRequest.RequestParser()
        self.connector = connector # 底层网络连接
        self.connector.setChannelObj(self)
        self.controller = RpcController(self) # 传递channel给上层
        self.listeners = set()

        self.logger = LogManager.getLogger('MarsRpc.RpcChannel')
        self.logger.info('RpcChannel.__init__: an new connection')

        # user data
        self.userData = None
        self.encrypted = False
        self.compressed = False
        self.sessionSeed = None

    def setMaxDataBytes(self, maxBytes):
        self.rpcRequestParser.setMaxDataBytes(maxBytes)

    def regListener(self, listener):
        """注册listener"""
        self.logger.debug('RpcChannel.regListener')
        self.listeners.add(listener)

    def unregListener(self, listener):
        self.listeners.discard(listener)

    def getPeername(self):
        """返回对端的(ip, port)"""
        assert self.connector is not None, 'No connector attached'
        return self.connector.getPeername()

    def setCrypter(self, encrypter, decrypter):
        if self.encrypted or self.compressed:
            self.connector.setCrypter(encrypter, decrypter)
        else:
            self.connector = CryptCompressConnector(self.connector, encrypter, decrypter)
        self.encrypted = True

    def setCompressor(self, compressor):
        if self.encrypted or self.compressed:
            self.connector.setCompressor(compressor)
        else:
            self.connector = CryptCompressConnector(self.connector, None, None, compressor)
        self.compressed = True

    def setUserData(self, userData):
        self.userData = userData

    def getUserData(self):
        return self.userData

    def setSessioSeed(self, seed):
        self.sessionSeed = seed

    def getSessionSeed(self):
        return self.sessionSeed

    def disconnect(self):
        """断开连接"""
        self.connector and self.connector.disconnect()

    def CallMethod(self, methodDescriptor, rpcController, request, responseClass, done):
        """发送rpc调用"""
        cmdIndex = methodDescriptor.index
        assert cmdIndex < 65535
        data = request.SerializeToString()
        totalLen = len(data) + 2
        self.connector.writeData(''.join([pack('<I', totalLen), pack('<H', cmdIndex), data]))

    def onDisconnected(self):
        """底层连接断开时回调"""
        self.logger.info('onDisconnected')
        for listener in self.listeners:
            listener.onChannelDisconnected(self)
        self.rpcRequest.reset()
        self.rpcRequestParser.reset()
        self.listeners = None
        self.connector = None
        self.userData = None

    def onRead(self, data):
        totalLen = len(data)
        skil = 0
        while skip < totalLen:
            result, consumBytes = self.rpcRequestParser.parse(self.rpcRequest, data, skip)
            assert consumBytes > 0

            skip += consumBytes
            if result == 1:
                ok = self.onRequest()
                self.rpcRequest.reset()

                if not ok:
                    return 0
                continue
            elif result == 0:
                return 0
            elif result == 3:
                raise ProtobufLengthError('buffer length > MAX_DATA_BYTES')
            else:
                continue

        return 2

    def onRequest(self):
        """解析一个完整请求后的回调"""
        dataLen = len(self.rpcRequest.data)

        if dataLen < 2:
            self.logger.error('got error request size: %d', dataLen)
            return False

        indexData = self.rpcRequest.data[0:2]
        cmdIndex = unpack('<H', indexData)[0]

        rpcService = self.rpcService
        descriptor = rpcService.GetDescriptor()

        if cmdIndex > len(descriptor.methods):
            self.logger.error('got error method inex: %d', cmdIndex)
            return False

        method = descriptor.methods[cmdIndex]
        try:
            request = rpcService.GetRequestClass(method)()
            serialized = self.rpcRequest.data[2:]
            request.ParseFromString(serialized)
            rpcService.CallMethod(method, self.controller, request, None)
        except:
            self.logger.error('call rpc method failed')
            self.logger.logLastExcept()

        return True
