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

import random
import sys
import traceback
import async_timer as _at
from common.nyx_codec import CodecDecoder
from common.nyx_entity import EntityFactory, EntityManager
from common.nyx_id import IdCreator
from common import nyx_rpc_utils as _rpcutils
from distributed.game import nyx_game_global as _gglobal
from proto.pyproto import nyx_common_pb2 as _c_pb2
from proto.pyproto import nyx_gate_game_pb2 as _gg_pb2
from proto.pyproto import nyx_game_manager_pb2 as _gm_pb2
from log.nyx_log import LogManager
from server.nyx_client_proxy import ClientProxy
from server.nyx_server_entity import AvatarEntity, PrepareEntity, PostEntity

class GameClientProxy(object):
    def __init__(self, stub):
        super(GameClientProxy, self).__init__()
        # TODO:
        pass

    def global_entity_message(self, global_entity_msg):
        # TODO:
        pass

    def forward_entity_message(self, entity_msg):
        # TODO:
        pass

    def create_real_entity(self, create_info):
        # TODO:
        pass

    def real_entity_created(self, entity_mailbox):
        # TODO:
        pass

    def bind_client_to_game(self, bindmsg):
        # TODO:
        pass

class GameClientProxyManager(object):
    def __init__(self):
        super(GameClientProxyManager, self).__init__()
        # TODO:
        pass

    def add_gate_proxy(self, proxy, gate_id=None):
        # TODO:
        pass

    def del_gate_proxy(self, gate_id):
        # TODO:
        pass

    def get_random_gate_proxy(self):
        # TODO:
        pass

    def has_gate_proxy(self, gate_id):
        # TODO:
        pass

    def get_gate_proxies(self):
        # TODO:
        pass

class GameService(_gg_pb2.SGateToGame):
    def __init__(self, config, config_sections):
        super(GameService, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Game.GameService')
        # TODO:
        pass

    def _create_entity(self, entity_name):
        # TODO:
        pass

    def _parse_client_info(self, msg, client_info):
        # TODO:
        pass

    def _get_new_client_proxy(self, stub, client_info):
        # TODO:
        pass

    def _on_game_traceback(self):
        # TODO:
        pass

    def connect_server(self, controller, request_msg, done):
        """客户端连接服务器rpc调用"""
        # TODO:
        pass

    def _do_bind_avatar(self, client_info, request_msg, rpc_channel):
        # TODO:
        pass

    def _do_new_connection(self, client_info, rpc_channel):
        # TODO:
        pass

    def _do_reconnection(self, client_info, request_msg, rpc_channel):
        # TODO:
        pass

    def _reconnection_failed(self, client_info, rpc_channel):
        # TODO:
        pass

    def _get_server_rpc_method(self, entity, method):
        # TODO: 获取entity的rpc方法
        pass

    def _get_rpc_method(self, method):
        # TODO:
        pass

    def is_entity_id_valid(self, client_proxy, entity_id):
        # TODO:
        pass

    def reg_md5index(self, controller, md5_index, done):
        # TODO:
        pass

    def entity_message(self, controller, entity_msg, done):
        # TODO:
        pass

    def _call_entity_method(self, entity_id, entity_msg):
        # TODO:
        pass

    def forward_entity_message(self, controller, entity_msg, done):
        """gate转发过来的entity的rpc消息调用"""
        # TODO:
        pass

    def reg_gate_server(self, controller, info, done):
        # TODO:
        pass

    def client_lose_connection(self, controller, client_info, done):
        """客户端断开连接的消息rpc通知"""
        # TODO:
        pass

    def get_entity_method(self, entity, method):
        """获取entity的rpc方法"""
        # TODO:
        pass

    def on_transfer_client(self, controller, return_value, done):
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

    def _del_clients_binding_of_gate(self, gate_id):
        # TODO:
        pass

    def gate_callback(self, controller, return_value, done):
        # TODO:
        pass

    def unbind_client_from_game(self, controller, client_info, done):
        # TODO:
        pass

    def _remove_client_proxy(self, client_id):
        # TODO: 删除一个client proxy
        pass

    def create_real_entity(self, controller, create_info, done):
        # TODO:
        pass

    def real_entity_created(self, controller, entity_mailbox, done):
        # TODO:
        pass

    def update_load(self):
        # TODO:
        pass
