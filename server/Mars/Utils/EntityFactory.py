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

import inspect
from MarsLog.LogManager import LogManager
from MessageCodec import Md5IndexDecoder
from Utils import ExtendableType

class EntityFactory(object):
    __metaclass__ = ExtendableType
    _instance = None

    def __init__(self):
        self.logger = LogManager.getLogger('Utils.EntityFactory')
        self.entityClasses = {}

    @staticmethod
    def instance(cls):
        if cls._instance == None:
            cls._instance = EntityFactory()
        return cls._instance

    def registerEntity(self, entityType, entityClass):
        self.entityClasses[entityType] = entityClass
        Md5IndexDecoder.registerStr(entityClass.__name__)
        methods = inspect.getmembers(entityClass, predicate=inspect.ismethod)
        methods.sort(lambda a, b: cmp(a[0], b[0]))
        for method in methods:
            if not method[0].startwith('_'):
                Md5IndexDecoder.registerStr(method[0])

    def getEntityClass(self, entityType):
        EntityClass = None
        if isinstance(entityType, str):
            EntityClass = self.entityClasses.get(entityType, None)
        elif isinstance(entityType, type):
            EntityClass = entityType
        return EntityClass

    def createEntity(self, entityType, entityId=None):
        EntityClass = self.getEntityClass(entityType)
        if not EntityClass:
            self.logger.error('failed to create entity for type %s id %s', str(entityType), str(entityId))
            return None
        if entityId:
            return EntityClass(entityId)
        else:
            return EntityClass()
