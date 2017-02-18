#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2017 ASMlover. All rights reserved.
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

from log.LogManager import LogManager
from nyx_common import ExtendableType

class EntityFactory(object):
    __metaclass__ = ExtendableType
    _instace = None

    def __init__(self):
        self._logger = LogManager.get_logger('NyxCore.EntityFactory')
        self._entity_classes = {}

    @staticmethod
    def get_instance(cls):
        if cls._instace is None:
            cls._instace = EntityFactory()
        return cls._instace

    def register_entity(self, entity_type, entity_class):
        self._entity_classes[entity_type] = entity_class
        # TODO:
        import inspect
        methods = inspect.getmembers(entity_class, predicate=inspect.ismethod)
        methods.sort(lambda a, b: cmp(a[0], b[0]))
        for method in methods:
            if not method[0].startswith('_'):
                # TODO: need FIXME:
                pass

    def get_entity_class(self, entity_type):
        entity_class = None
        if isinstance(entity_type, str):
            entity_class = self._entity_classes.get(entity_type)
        elif isinstance(entity_type, type):
            entity_class = entity_type
        return entity_class

    def create_entity(self, entity_type, entity_id=None):
        entity_class = self.get_entity_class(entity_type)
        if not entity_class:
            self._logger.error('create entity(type:%s, id:%s) failed', entity_type, entity_id)
            return None
        if entity_id is None:
            return entity_class()
        else:
            return entity_class(entity_id)
