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
from msgpack.bMsgPack import msgPackExt, extHook
import msgpack
from MarsLog.LogManager import LogManager
from Utils.IdCreator import IdCreator
from Utils.PyProto.Common_pb2 import EntityRpc

class ServerProxy(object):
    def __init__(self, stub, encoder, proto='BSON'):
        super(ServerProxy, self).__init__()
        self.stub = stub
        stub.rpcChannel.regListener(self)
        self.owner = None
        self.logger = LogManager.getLogger('Client.ServerProxy')
        self.encoder = encoder
        self.proto = proto

    def setOwner(self, owner):
        self.owner = owner

    def getOwnerId(self):
        return self.owner and self.owner.eid else None

    def sendRegMd5Index(self, md5Index):
        self.stub.regMd5Index(None, md5Index)

    def destroy(self):
        self.stub.rpcChannel.unregListener(self)
        self.stub = None
        self.owner = None
        self.encoder = None

    def isCachable(self):
        return False

    def callServerMethod(self, methodName, parameters=None, entityId=None):
        if entityId is None:
            entityId = self.getOwnerId()
            if entityId is None:
                self.logger.error('callServerMethod: did not find owner %s, need pass entityId to call %s', str(self.owner), methodName)
                return
        entityRpc = EntityRpc()
        entityRpc.id = IdCreator.id2bytes(entityId)
        self.encoder.encode(entityRpc.method, methodName)
        if parameters is not None:
            if self.proto == 'BSON' or self.proto == 'bson':
                entityRpc.arguments = BSON.encode(parameters)
            elif self.proto == 'msgpack':
                entityRpc.arguments = msgpack.packb(parameters, use_bin_type=True, default=msgPackExt)
        self.stub.entityRpc(None, entityRpc)

    def onChannelDisconnected(self, rpcChannel):
        self.owner.onLoseServer()
        self.owner.setServer(None)
        self.owner = None
        self.destroy()
