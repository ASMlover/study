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

import os
from Common import Common
from Common.Codec import CodecDecoder
from Common.Common import ExtendableType
from Log.LogManager import LogManager

class EntityFactory(object):
    __metaclass__ = ExtendableType
    _instance = None

    def __init__(self):
        self._logger = LogManager.get_logger('Common.EntityFactory')
        self._decoder = CodecDecoder()
        self._entity_classes = {}

    @staticmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = EntityFactory()
        return cls._instance

    def reg_entity(self, entity_type, entity_class):
        import inspect

        self._entity_classes[entity_type] = entity_class
        self._decoder.reg_str(entity_class.__name__)
        methods = inspect.getmembers(entity_class, predicate=inspect.ismethod)
        for name, _ in methods:
            if not name.startswith('_'):
                self._decoder.reg_str(name)

    def get_entity_class(self, entity_type):
        entity_class = None
        if isinstance(entity_type, str):
            entity_class = self._entity_classes.get(entity_type)
        elif isinstance(entity_type, type):
            entity_class = entity_type
        return entity_class

    def create_entity(self, entity_type, entity_id=None):
        entity_class = self.get_entity_class(entity_type)
        if entity_class is None:
            self._logger.error(
                    'EntityFactory.create_entity - {type:%s, id:%s} failed',
                    entity_type, entity_id)
            return None
        if entity_id is None:
            return entity_class()
        else:
            return entity_class(entity_id)

class EntityManager(object):
    """管理所有entity的类"""
    _logger = LogManager.get_logger('Common.EntityManager')
    _entities = {}

    @staticmethod
    def has_entity(entity_id):
        return entity_id in EntityManager._entities

    @staticmethod
    def get_entity(entity_id):
        return EntityManager._entities.get(entity_id)

    @staticmethod
    def add_entity(entity_id, entity, overwrite=False):
        if entity_id in EntityManager._entities:
            EntityManager._logger.warn(
                    'EntityManager.add_entity - entity(%s) already exists',
                    entity_id)
            if not overwrite:
                return
        EntityManager._entities[entity_id] = entity

    @staticmethod
    def del_entity(self, entity_id):
        EntityManager.pop(entity_id, None)

class EntityScanner(object):
    _logger = LogManager.get_logger('Common.EntityScanner')

    @staticmethod
    def _get_module_names(module_dir):
        module_names = set()
        try:
            module_files = os.listdir(module_dir)
        except:
            EntityScanner._logger.warn(
                    'EntityScanner._get_module_names - (%s) failed',
                    module_dir)
            return ()

        for fname in module_files:
            splited_list = fname.split('.')
            if len(splited_list) == 2:
                if splited_list[1] in ('py', 'pyc', 'pyo'):
                    module_names.add(splited_list[0])
        module_names.discard('__init__')
        return module_names

    @staticmethod
    def _get_modules(module_dir):
        module_names = EntityScanner._get_module_names(module_dir)
        modules = []
        Common.insert_syspath(module_dir)
        for module_name in module_names:
            try:
                mod = __import__(module_name, fromlist=[''])
                if mod:
                    modules.append(mod)
            except:
                EntityScanner._logger.warn(
                        'EntityScanner._get_modules - (%s) failed',
                        module_name)

                import traceback
                traceback.print_exc()
                continue
        return modules

    @staticmethod
    def _get_classes(module, base_classes):
        """获取module模块中属于base_classes子类的类"""
        classes = []
        for name in dir(module):
            cls = getattr(module, name)
            if isinstance(cls, type) and isinstance(cls, base_classes):
                classes.append(cls)
        return classes

    @staticmethod
    def scan_entity_classes(module_dir, base_classes):
        class_dict = {}
        for module in EntityScanner._get_modules(module_dir):
            classes = EntityScanner._get_classes(module, base_classes)
            for cls in classes:
                class_dict[cls.__name__] = cls
        return class_dict
