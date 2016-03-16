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

class EntityManager(object):
    _logger = LogManager.getLogger('Utils.EntityManager')
    _entities = {}

    @staticmethod
    def hasEntity(entityId):
        return entityId in EntityManager._entities

    @staticmethod
    def getEntity(entityId):
        return EntityManager._entities.get(entityId)

    @staticmethod
    def addEntity(entityId, entity, override=False):
        if entityId in EntityManager._entities:
            EntityManager._logger.warn('entity %s already exists', entityId)
            if not override:
                return
        EntityManager._entities[entityId] = entity
        EntityManager._logger.info('addEntity entity id %s', entityId)

    @staticmethod
    def delEntity(entityId):
        try:
            del EntityManager._entities[entityId]
        except KeyError:
            EntityManager._logger.warn('entity id %s did not exists', entityId)
        EntityManager._logger.info('delEntity entity id %s', entityId)
