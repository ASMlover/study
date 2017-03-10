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

from bson import Binary
from common.pyproto import nyx_common_pb2 as _c_pb2
from common.pyproto import nyx_gate_game_pb2 as _gg_pb2
from common.pyproto import nyx_db_manager_pb2 as _dm_pb2
from common.pyproto import nyx_game_manager_pb2 as _gm_pb2
from common import nyx_common as _nc
from common.nyx_codec import CodecEncoder
from common.nyx_entity import EntityFactory
from common.nyx_entity import EntityManager
from common.nyx_id import IdCreator
from log.nyx_log import LogManager
from distributed.game import nyx_game_global as _gglobal

_codec_encoder = CodecEncoder()
_logger = LogManager.get_logger('NyxCore.Game.GameAPI')

def _create_remote_entity(entity_type, server_info=None, entity_id=None,
        entity_content=None, fromdb=False, transfer_entity=False, client_info=None, callback=None):
    """在任意一个远程game server上创建entity"""
    if not _gglobal.nyx_gamemgr_proxy.connected:
        _logger.error('_create_remote_entity: game lost connection with game manager')
        return

    entity_info_header = _gm_pb2.EntityInfoHeader()
    if server_info is None:
        entity_info_header.create_anywhere = _gm_pb2.EntityInfoHeader.ANY_WHERE
    else:
        entity_info_header.dst_server.CopyFrom(server_info)
        entity_info_header.create_anywhere = _gm_pb2.EntityInfoHeader.SPECIFY_SERVER
    entity_info_header.trans_entity = transfer_entity
    entity_info_header.create_fromdb = fromdb
    if callback is not None:
        entity_info_header.callback_id = _gglobal.reg_gamemgr_callback(callback)
    if client_info is not None:
        entity_info_header.client_info.CopyFrom(client_info)
    entity_info = _c_pb2.EntityInfo()
    entity_info.routes = entity_info_header.SerializeToString()
    _codec_encoder.encode(entity_info.type, entity_type)
    if entity_id is not None:
        entity_info.entity_id = entity_id
    if entity_content is not None:
        entity_info.info = _gglobal.proto_encoder.encode(entity_content)
    _gglobal.nyx_gamemgr_proxy.create_entity(entity_info)

def call_global_client_method(method_name, parameters, reliable=True):
    """server ->(call)-> client"""
    global_entity_msg = _c_pb2.GlobalEntityRpcMessage()
    _codec_encoder.encode(global_entity_msg.method, method_name)
    global_entity_msg.params = _gglobal.proto_encoder.encode(parameters)
    global_entity_msg.reliable = reliable
    _gglobal.nyx_gamemgr_proxy.global_entity_message(global_entity_msg)

def call_global_group_client_method(target, method_name, parameters, reliable=True):
    """server ->(call)-> client"""
    global_entity_msg = _c_pb2.GlobalEntityRpcMessage()
    global_entity_msg.target = _gglobal.proto_encoder.encode(target)
    _codec_encoder.encode(global_entity_msg.method, method_name)
    global_entity_msg.params = _gglobal.proto_encoder.encode(parameters)
    global_entity_msg.reliable =  reliable
    _gglobal.nyx_gamemgr_proxy.global_entity_message(global_entity_msg)
