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

from Utils.EntityManager import EntityManager
from Utils.IdCreator import IdCreator
from Utils.RpcDecorator import rpcMethod, MARS_RPC_CLIENT_STUB
from MarsLog.LogManager import LogManager

class ClientEntity(object):
    def __init__(self, entityId=None):
        super(ClientEntity, self).__init__()
        self.logger = LogManager.getLogger('ClientEntity.%s' % self.__class__.__name__)
        self.logger.info('ClientEntity.__init__')
        self.eid = (entityId is None) and IdCreator.genId() or entityId
        EntityManager.addEntity(self.eid, self, False)
        self.server = None

    def setServer(self, server):
        self.server = server

    def onEntityCreation(self):
        """通知Client这就是player"""
        self.logger.info('ClientEntity.onEntityCreation: eid=%s', self.eid)

    def onLoseServer(self):
        """与Server断开连接"""
        self.logger.info('ClientEntity.onLoseServer: eid=%s', self.eid)

    def onDestroy(self):
        pass

    def initFromDict(self, dictData):
        pass

    def destroy(self):
        self.onDestroy()
        if self.server:
            self.server.destroy()
            self.server = None
        self.logger = None
        EntityManager.delEntity(self.eid)

    def onReconnected(self, extraData):
        """重连成功"""
        self.logger.info('ClientEntity.onReconnected: eid=%s, extraData=%s', self.eid, extraData)

class AvatarEntity(ClientEntity):
    def __init__(self, entityId=None):
        super(AvatarEntity, self).__init__(entityId)

    @rpcMethod(MARS_RPC_CLIENT_STUB)
    def becomePlayer(self):
        self.onBecomePlayer()

    def onBecomePlayer(self):
        pass
