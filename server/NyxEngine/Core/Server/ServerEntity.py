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

import async_timer
from Common.Codec import Md5Cache
from Common.EntityUtils import EntityFactory, EntityManager
from Common.IdUtils import IdUtils
from Log.LogManager import LogManager
from Server.ClientProxy import ClientProxy

class DirtyManager(object):
    _dirties = {}

    @staticmethod
    def add_dirty_state(id, is_dirty):
        DirtyManager._dirties[id] = is_dirty

    @staticmethod
    def del_dirty_state(id):
        DirtyManager._dirties.pop(id, None)

    @staticmethod
    def set_dirty_state(id, is_dirty):
        if id in DirtyManager._dirties:
            DirtyManager._dirties[id] = is_dirty

    @staticmethod
    def get_dirty_state(id):
        return DirtyManager._dirties.get(id, False)

class EntityProxy(object):
    """服务端Entity的代理"""
    def __init__(self, entity, mailbox):
        self.entity = entity
        self.mailbox = mailbox

    def __getattr__(self, name):
        def _caller(*args):
            if args:
                parameters = {'_': args}
            else:
                parameters = None
            self.entity.call_server_method(self.mailbox, name, parameters)
        return _caller

class ServerEntity(object):
    """所有服务器对象的基类"""
    def __init__(self, entity_id=None):
        super(ServerEntity, self).__init__()
        self.logger = LogManager.get_logger(
                'ServerEntity.%s' % self.__class__.__name__)
        if entity_id is None:
            self.entity_id = IdUtils.gen_id()
        else:
            self.entity_id = entity_id
        self.gate_proxy = None
        self.save_timer = None
        self.is_destroyed = False
        save_interval = self.get_persistent_time()
        if save_interval > 0:
            self.save_timer = async_timer.add_cycle_timer(
                    save_interval, lambda: self.save())
        EntityManager.add_entity(self.entity_id, self, False)
        DirtyManager.add_dirty_state(self, False)

    def __getattribute__(self, key):
        if key != '__class__':
            DirtyManager.set_dirty_state(self, True)
        return super(ServerEntity, self).__getattribute__(key)

    def tick(self):
        """每Game Tick的时间调用一次"""
        pass

    def save(self, callback=None):
        if self.is_persistent():
            if DirtyManager.get_dirty_state(self):
                # TODO: need implementation GameAPI operations
                # GameAPI.save_entity(self, callback)
                DirtyManager.set_dirty_state(self, False)
        elif callback:
            callback(True)

    def cancel_save_timer(self):
        if self.save_timer:
            self.save_timer.cancel()
            self.save_timer = None

    def _destroy_callback(self, r, callback):
        callback(r)
        EntityManager.del_entity(self.entity_id)

    def destroy(self, callback=None):
        """销毁当前Entity"""
        if self.is_destroyed:
            return
        self.cancel_save_timer()
        if callback is None:
            self.save()
            EntityManager.del_entity(self.entity_id)
        else:
            self.save(lambda r: self._destroy_callback(r, callback))
        DirtyManager.del_dirty_state(self)
        self.is_destroyed = True
