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

from MarsRpc.LogManager import LogManager
from RpcChannel import RpcChannel

class RpcChannelCreator(object):
    def __init__(self, rpcService, channelHandler, maxDataBytes=0):
        super(RpcChannelCreator, self).__init__()
        self.logger = LogManager.getLogger('MarsRpc.RpcChannelCreator')
        self.rpcService = rpcService
        self.channelHandler = channelHandler
        self.maxDataBytes = maxDataBytes

    def setMaxDataBytes(self, maxBytes):
        self.maxDataBytes = maxBytes

    def handleNewConnector(self, connector):
        self.logger.info('handle new connector %s', connector.socket.getpeername())
        rpcChannel = RpcChannel(self.rpcService, connector)
        if self.maxDataBytes > 0:
            rpcChannel.setMaxDataBytes(self.maxDataBytes)
        self.channelHandler.handleNewChannel(rpcChannel)

    def handleConnectorFailed(self, connector):
        self.logger.info('connector failed')

class RpcChannelHolder(object):
    """管理多个RpcChannel"""
    def __init__(self):
        super(RpcChannelHolder, self).__init__()
        self.logger = LogManager.getLogger('MarsRpc.RpcChannelHolder')
        self.rpcChannel = None

    def getRpcChannel(self):
        return self.rpcChannel

    def handleNewChannel(self, rpcChannel):
        self.logger.debug('RpcChannelHolder handleNewChannel')
        self.rpcChannel = rpcChannel
        rpcChannel.regListener(self)

    def onChannelDisconnected(self, rpcChannel):
        self.rpcChannel = None

class RpcChannelManager(object):
    """管理所有的RpcChannel"""
    def __init__(self):
        super(RpcChannelManager, self).__init__()
        self.logger = LogManager.getLogger('MarsRpc.RpcChannelManager')
        self.rpcChannels = {}

    def getRpcChannel(self, peername):
        return self.rpcChannels.get(peername, None)

    def rpcChannelCount(self):
        return len(self.rpcChannels)

    def handleNewChannel(self, rpcChannel):
        self.logger.debug('RpcChannelManager handleNewChannel')
        self.rpcChannels[rpcChannel.getPeername()] = rpcChannel
        rpcChannel.regListener(self)

    def onChannelDisconnected(self, rpcChannel):
        peername = rpcChannel.getPeername()
        if peername in self.rpcChannels:
            self.logger.info('delete connector fro %s', peername)
            self.rpcChannels.pop(peername, None)
        else:
            self.logger.info('do not find disconnected connector %s', peername)
