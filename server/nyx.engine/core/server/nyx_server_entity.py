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
        mailbox = _gutils.get_global_entity_mailbox(name)
        if mailbox is not None:
            return EntityProxy(self, mailbox)
        return self.get_global_entity(name)

    def get_global_entity(self, name):
        mailbox = _gutils.get_global_entity_mailbox(name)
        if mailbox is not None:
            return EntityProxy(self, mailbox)

        entity = _gutils.create_entity_locally(name)
        def _failed_callback(success):
            if not success:
                entity.destroy()
        _gutils.register_entity_globally(name, entity, _failed_callback)
        return EntityProxy(self, entity)

    def on_tick(self):
        """每个game tick的回调"""
        pass

    def save(self, callback=None):
        if self.is_persistent():
            if DirtyManager.get_dirty_state(self):
                _gutils.save_entity(self, callback)
                DirtyManager.set_dirty_state(self, False)
        else:
            callback and callback(True)

    def destroy(self, callback=None):
        """销毁自己"""
        if self.is_destroyed:
            return
        self.cancel_save_timer()
        if callback is None:
            self.save()
            EntityManager.get_instance().del_entity(self.id)
        else:
            self.save(lambda r: self.destroy_callback(r, callback))
        self.is_destroyed = True
        DirtyManager.del_dirty_state(self)

    def cancel_save_timer(self):
        if self._save_timer:
            self._save_timer.cancel()
            self._save_timer = None

    def destroy_callback(self, result, callback):
        callback(result)
        EntityManager.get_instance().del_entity(self.id)

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

    def _get_gate_proxy(self):
        """获取对于的game proxy"""
        # TODO: game service 上需要实现 get_random_gate_proxy()接口
        if (self._gate_proxy is None or
                not _gglobal.nyx_game_client_mgr.has_gate_proxy(self._gate_proxy.gate_id)):
            self._gate_proxy = _gglobal.nyx_game_client_mgr.get_random_gate_proxy()
        return self._gate_proxy

    def call_server_method(self, remote_mailbox, method, parameters=None, callback=None):
        """调用其他game上entity的rpc方法"""

    def transfer_to_server(self, dst_server, content=None, callback=None):
        """迁移到其他server

        在dst_server创建预备entity，检查与目标服务器是否能正确迁移；
        将entity迁移到dst_server，需要注意有些entity有数据保存的需要做数据库迁移；
        在src_server创建事后entity，负责转发server到server的rpc
        """

    def _transfer_result(self, status, user_callback):
        pass

    def _create_pre_entity_callback(self, status, dst_server, pre_entity_id, content, user_callback):
        pass

    def _create_post_entity(self, dst_server, bindmsg=None, client=None):
        pass

    def _do_transfer(self, dst_server, pre_entity_id, content, user_callback):
        pass

    def _get_client_info(self):
        return None

    def pre_reload_script(self):
        pass

    def reload_script(self):
        pass

    def on_server_closing(self):
        """通知服务器将要关闭"""
        pass

    def on_server_closed(self, callback=None):
        """通知服务器关闭"""
        pass

    def on_reconnected(self, auth_msg=None):
        """重连成功"""
        pass

    def can_reconnected(self, auth_msg):
        """是否可以重连"""
        return self.client is not None

    def get_reconnected_extra(self):
        """获取重连是额外传递给客户端的信息"""
        return {}

    def copy_gate_proxy_to(self, other):
        """将gate proxy拷贝给另一个entity"""
        other._gate_proxy = self._gate_proxy

    def get_mailbox(self):
        mailbox = _c_pb2.EntityMailbox()
        mailbox.entity_id = self.id
        mailbox.server_info.CopyFrom(_gglobal.nyx_game_info)
        return mailbox

class PrepareEntity(ServerEntity):
    """预备entity，用于迁移时在目标服建立的entity"""
    _LIFE_CYCLE = 20
    def __init__(self, entity_id=None):
        pass

    def create_real_entity(self, create_info):
        pass

    def _create_fromdb_callback(self, create_info, real):
        pass

    def _create_client_proxy(self, entity, create_info):
        """创建entity的client proxy"""
        pass

    def _handle_destroy(self):
        pass

    def destroy(self, callback=None):
        pass

class PostEntity(ServerEntity):
    """迁移后，留在原服务器用于转发的entity"""
    _LIFE_CYCLE = 20
    _MAX_CACHE_MESSAGE = 2000
    def __init__(self, entity_id=None):
        pass

    def set_dst_server(self, dst_server):
        pass

    def set_bind_client(self, client):
        pass

    def set_bind_client_msg(self, bindmsg):
        pass

    def forward_message(self, entity_msg):
        """转发消息"""
        pass

    def _do_forward_message(self, entity_msg):
        pass

    def real_entity_created(self):
        pass

    def _bind_client(self):
        pass

    def _flush(self):
        pass

    def _handle_destroy(self):
        pass

    def destroy(self, callback=None):
        pass
