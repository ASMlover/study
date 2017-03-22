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

from common.nyx_id import IdCreator
from distributed.gate import nyx_gate_global as _gglobal
from log.nyx_log import LogManager
from rpc.nyx_channel_client import ChannelClient
from proto.pyproto import nyx_common_pb2 as _c_pb2
from proto.pyproto import nyx_gate_game_pb2 as _gg_pb2
from proto.pyproto import nyx_game_manager_pb2 as _gm_pb2
from proto.pyproto.nyx_common_pb2 import EntityMailbox

class MessageCachedProxy(object):
    """消息缓存proxy"""
    _CONNECT_SERVER = 1
    _ENTITY_MESSAGE = 2
    _LOSE_CONNECTION = 3

    def __init__(self, client_id):
        super(MessageCachedProxy, self).__init__()

    def connect_server(self, client_info, request_msg):
        # TODO: 连接game服务器
        pass

    def entity_message(self, client_info, entity_msg):
        # TODO:
        pass

    def client_lose_connection(self, client_info):
        # TODO:
        pass

    def clear(self):
        # TODO:
        pass

    def flush_message(self, server_proxy):
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

def GameServerProxy(object):
    """gate同game连接的rpc client proxy"""
    def __init__(self, stub, server_info, gate_id):
        super(GameServerProxy, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Gate.GameServerProxy')
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

    def connect_server(self, client_info, request_msg):
        # TODO: 连接game服务器
        pass

    def entity_message(self, client_info, entity_msg):
        # TODO:
        pass

    def client_lose_connection(self, client_info):
        # TODO:
        pass

    def on_transfer_client(self, gate_return_value):
        # TODO:
        pass

    def forward_entity_message(self, entity_msg):
        # TODO:
        pass

    def unbind_client_from_game(self, client_info):
        # TODO:
        pass

    def gate_callback(self, callback_type, callback_id, return_value):
        # TODO:
        pass

    def reg_gate_server(self):
        # TODO:
        pass

    def create_real_entity(self, create_info):
        # TODO:
        pass

    def real_entity_created(self, entity_mailbox):
        # TODO:
        pass

    def reg_md5index(self, client_info, md5_index):
        # TODO:
        pass

class GameServerClient(_gg_pb2.SGateToGame):
    """gate与game的通信"""
    _STATUS_NOT_CONNECTED = 1
    _STATUS_CONNECTING = 2
    _STATUS_CONNECTED = 3

    def __init__(self, addr, port, proxy_mgr, gate_id, config, config_sections,
            ban_client=False, server_type=_c_pb2.ServerInfo.NORMAL):
        super(GameServerClient, self).__init__()
        self.logger = LogManager.get_logger('NyxCore.Game.GameServerClient')
        # TODO:
        pass

    def is_not_connected(self):
        return self.status == GameServerClient._STATUS_NOT_CONNECTED

    def start_connect(self, timeout):
        # TODO:
        pass

    def channel_callback(self, rpc_channel):
        # TODO:
        pass

    def on_channel_disconnected(self, rpc_channel):
        # TODO:
        pass

    def _get_proxy(self, routes):
        # TODO:
        pass

    def on_connect_server(self, controller, response, done):
        # TODO:
        pass

    def create_entity(self, controller, entity_info, done):
        # TODO:
        pass

    def destroy_entity(self, controller, entity_info, done):
        # TODO:
        pass

    def disconnect_client(self, controller, client_info, done):
        # TODO:
        pass

    def entity_message(self, controller, entity_msg, done):
        # TODO:
        pass

    def global_entity_message(self, controller, entity_msg, done):
        # TODO:
        pass

    def forward_entity_message(self, controller, entity_msg, done):
        # TODO:
        pass

    def chat_to_client(self, controller, outband_info, done):
        # TODO:
        pass

    def _do_transfer_client(self, client_info, dst_server):
        # TODO:
        pass

    def  transfer_client(self, controller, client_bindmsg, done):
        # TODO:
        pass

    def bind_client_to_game(self, controller, client_bindmsg, done):
        # TODO:
        pass

    def create_real_entity(self, controller, create_info, done):
        # TODO:
        pass

    def real_entity_created(self, controller, entity_mailbox, done):
        # TODO:
        pass

    def reg_md5index(self, controller, md5_index, done):
        # TODO:
        pass

    def update_game_load(self, controller, game_load, done):
        # TODO:
        pass

    def gate_method(self, controller, method, done):
        # TODO:
        pass
