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

import time
import async_timer as _at
from common.nyx_entity import EntityFactory, EntityManager
from common.nyx_codec import Md5Cache
from common.nyx_id import IdCreator
from proto.pyproto import nyx_common_pb2 as _c_pb2
from proto.pyproto import nyx_gate_game_pb2 as _gg_pb2
from proto.pyproto import nyx_game_manager_pb2 as _gm_pb2
from log.nyx_log import LogManager
from distributed.game import nyx_game_global as _gglobal
from distributed.game import nyx_game_utils as _gutils
from server.nyx_client_proxy import ClientProxy

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
    def __init__(self, entity_id=None):
        self.logger = LogManager.get_logger('NyxCore.ServerEntity.%s' % self.__class__.__name__)
        self.id = entity_id or IdCreator.gen_id()
        self._gate_proxy = None
        self._save_timer = None
        EntityManager.get_instance().add_entity(self.id, self, False)
        self.is_destroyed = False
        save_time = self.get_persistent_time()
        if save_time > 0:
            self._save_timer = _at.add_cycle_timer(save_time, lambda: self.save())
        DirtyManager.add_dirty_state(self, False)

    def __getattribute__(self, key):
        if key == '__class__':
            return super(ServerEntity, self).__getattribute__(key)
        DirtyManager.set_dirty_state(self, True)
        return super(ServerEntity, self).__getattribute__(key)

    def create_global_entity(self, name):
        pass

    def get_global_entity(self, name):
        pass

    def on_tick(self):
        """每个game tick的回调"""
        pass

    def save(self, callback=None):
        pass

    def destroy(self, callback=None):
        """销毁自己"""
        pass

    def cancel_save_timer(self):
        pass

    def destroy_callback(self, result, callback):
        pass

    def is_persistent(self):
        return False

    def get_persistent_time(self):
        return _gglobal.nyx_def_savetime

    def init_from_dict(self, entity_dict):
        """以entity_dict数据来初始化字节"""
        pass

    def get_persistent_dict(self):
        """获取可持久化的数据信息"""
        return {}
