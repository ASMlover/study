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

import sys
from bson import BSON
from msgpack.bMsgPack import msgPackExt, extHook
import msgpack

from MarsLog.LogManager import LogManager
from MarsRpc.ChannelClient import ChannelClient
from MarsRpc.Compressor import Compressor

from Utils.PyProto import ClientGate_pb2
from Utils.PyProto import Common_pb2
from Utils.EntityFactory import EntityFactory
from Utils.EntityManager import EntityManager
from Utils.IdCreator import IdCreator
from Utils.MessageCodec import Md5IndexDecoder, Md5IndexEncoder

from ServerProxy import ServerProxy

MARS_DEVICEID = str(IdCreator.genId())

class GateClient(ClientGate_pb2.SGate2Client):
    ST_INIT                 = 0
    ST_CONNECTING           = 1
    ST_RECONNECTING         = 3
    ST_CONNECT_FAILED       = 4
    ST_CONNECT_SUCCESSED    = 5
    ST_DISCONNECTED         = 6

    CB_ON_CONNECT_FAILED        = 1
    CB_ON_CONNECT_SUCCESSED     = 2
    CB_ON_DISCONNECTED          = 3
    CB_ON_CONNECT_REPLY         = 4
    CB_ON_RELIABLE_MSG_UNSENT   = 5

    def __init__(self, host, port, conf, proto='BSON'):
        super(GateClient, self).__init__(self)
        self.logger = LogManager.getLogger('Client.GateClient')
        self.client = ChannelClient(host, port, self)
        self.status = GateClient.ST_INIT
        self.reconnectStatus = False
        self.reconnectData = None
        self.gateStub = None
        self.encoder = Md5IndexEncoder()

        # traceback
        self.tbHandler = None
        self.proto = proto
        enforceEncryption = bool(conf.get('enforceEncryption', False))
        loginKeyPath = conf.get('loginKeyPath', None)
        loginKeyContent = conf.get('loginKeyContent', None)
        useKeyczar = bool(conf.get('useKeyczar', None))
        self.zippedEnabled = bool(conf.get('zippedEnabled', False))
        self.enableRegMd5Index = bool(conf.get('enableRegMd5Index', False))

        self.receivedSeq = 0
        self.useMessageCache = False
        self.sessionPaddingMinLen = 16
        self.sessionPaddingMaxLen = 64

        self.connectStatus = Common_pb2.ConnectReply.RT_BUSY
        self.sessionSeed = ClientGate_pb2.SessionSeed()
        if enforceEncryption:
            if useKeyczar:
                from Utils.SessionEncrypter import LoginKeyEncrypter
                self.keyEncrypter = LoginKeyEncrypter(loginKeyPath)
            else:
                from Utils.SessionEncrypter import LoginKeyEncrypterNoKeyczar
                self.keyEncrypter = LoginKeyEncrypterNoKeyczar(loginKeyPath, loginKeyContent)
        else:
            self.keyEncrypter = None

        self.onEventCallbacks = {
            GateClient.CB_ON_CONNECT_FAILED:        set(),
            GateClient.CB_ON_CONNECT_SUCCESSED:     set(),
            GateClient.CB_ON_DISCONNECTED:          set(),
            GateClient.CB_ON_CONNECT_REPLY:         set(),
            GateClient.CB_ON_RELIABLE_MSG_UNSENT:   set(),
        }

    def enableMessageCache(self, enabled=True):
        self.useMessageCache = enabled

    def setSessionPaddingLength(self, minLen, maxLen):
        self.sessionPaddingMinLen = minLen
        self.sessionPaddingMaxLen = maxLen

    def startGame(self, timeout):
        self.encoder.reset()
        self.receivedSeq = 0
        assert self.status in (GateClient.ST_INIT, GateClient.ST_DISCONNECTED, GateClient.ST_CONNECT_FAILED), 'startGame: status is wrong, status=%s' % self.status
        self.status = GateClient.ST_CONNECTING
        self.doConnect(self.channelCallback, timeout)

    def resumeGame(self, timeout, entityId, binAuthMsg):
        pass

    def doConnect(self, channelCallback, timeout):
        pass

    def disconnect(self):
        pass

    def channelCallback(self, rpcChannel):
        pass
