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

from bson import BSON
from msgpack.bMsgPack import extHook, msgPackExt
import msgpack
from MarsLog.LogManager import LogManager
from Utils.IdCreator import IdCreator
from Utils.PyProto.Common_pb2 import EntityRpc

class CachedServerProxy(object):
    OP_SEND_MSG_MD5_INDEX = 0
    OP_CALL_SERVER_METHOD = 1

    def __init__(self, stub, encoder, proto='BSON'):
        super(CachedServerProxy, self).__init__()
        self.logger = LogManager.getLogger('Client.CachedServerProxy')
        self.stub = None
        self.owner = None
        self.encoder = encoder
        self.reliableMsgCannotSentCB = None
        self.proto = proto
        self.dispatchMap = {}
        self.setStub(stub)

    def setStub(self, stub):
        self.stub = stub
        if stub is None:
            self.dispatchMap = {}
        else:
            stub.rpcChannel.regListener(self)
            self.dispatchMap = {
                self.OP_SEND_MSG_MD5_INDEX: self.stub.regMd5Index,
                self.OP_CALL_SERVER_METHOD: self.stub.entityRpc,
            }

    def setOwner(self, owner):
        self.logger.debug('CachedServerProxy.setOwner: %s', str(owner))
        self.owner = owner

    def sendRegMd5Index(self, md5Index):
        pass

    def callServerMethod(self, methodName, parameters=None, entityId=None, reliable=True):
        pass

    def getOwnerId(self):
        pass

    def onChannelDisconnected(self, rpcChannel):
        pass

    def destroy(self):
        pass

    def flushFromSeq(self, seq):
        pass

    def regReliableMessageCannotSendCB(self, cb):
        pass

    def isCachable(self):
        return True
