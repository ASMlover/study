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

import Timer

from MarsRpc.LogManager import LogManager
from RpcChannel import RpcChannel
from TcpClient import TcpClient

class ChannelClient(object):
    # CONNECT STATUS
    CS_INIT = 0
    CS_CONNECTING = 1
    CS_FAILED = 3
    CS_SUCCESSED = 4

    def __init__(self, ip, port, rpcService):
        super(ChannelClient, self).__init__()
        self.rpcService = rpcService
        self.ip = ip
        self.port = port
        self.logger = LogManager.getLogger('MarsRpc.ChannelClient')
        self.client = TcpClient(self.ip, self.port, self)
        self.status = ChannelClient.CS_INIT

        self.timer = None
        self.callback = None

    def getPeername(self):
        return self.client.getPeername()

    def reset(self, ip=None, port=None):
        self.client and self.client.close()
        self.ip = ip or self.ip
        self.port = port or self.port
        self.client = TcpClient(self.ip, self.port, self)
        self.status = ChannelClient.CS_INIT
        self.cancelTimer()

    def cancelTimer(self):
        if self.timer and not self.timer.cancelled and not self.timer.expired:
            self.timer.cancel()
            self.timer = None

    def _checkConnection(self):
        if self.status != ChannelClient.CS_SUCCESSED:
            self.callback and self.callback(None)
            self.status = ChannelClient.CS_FAILED
            self.logger.error('connection timeout')

    def connect(self, callback, timeout):
        self.client.asyncConnect()
        self.callback = callback
        self.status = ChannelClient.CS_CONNECTING
        self.timer = Timer.addTimer(timeout, self._checkConnection)

    def disconnect(self):
        self.client and self.client.disconnect()
