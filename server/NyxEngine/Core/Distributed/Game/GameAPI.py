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
from Common import Common
from Common.Codec import CodecEncoder
from Common.EntityUtils import EntityFactory, EntityManager
from Common.IdUtils import IdUtils
from Distributed.Game import GameGlobal
from Log.LogManager import LogManager
from Proto.Details import BasicProtocol_pb2 as _B_PB2
from Proto.Details import Common_pb2 as _C_PB2

_codec_encoder = CodecEncoder()
_logger = LogManager.get_logger('GameAPI')

def _create_remote_entity(entity_type, server_info=None,
        entity_id=None, entity_content=None, fromdb=False,
        transfer_entity=False, client_info=None, callback=None):
    """在任意指定的Game Server上创建Entity"""
    if not GameGlobal.gamemgr_proxy.connected:
        _logger.error(
                '_create_remote_entity: lost connection with game manager')
        return

    header = _B_PB2.EntityInfoHeader()
    if server_info is None:
        header.create_anywhere = _B_PB2.EntityInfoHeader.ANY_WHERE
    else:
        header.dst_server.CopyFrom(server_info)
        header.create_anywhere = _B_PB2.EntityInfoHeader.SPECIFY_SERVER
    header.transfer_entity = transfer_entity
    header.create_fromdb = fromdb
    if callback is not None:
        header.callback_id = GameGlobal.reg_gamemgr_callback(callback)
    if client_info is not None:
        header.client_info.CopyFrom(client_info)

    info = _C_PB2.EntityInfo()
    info.routes = header.SerializeToString()
    _codec_encoder.encode(info.type, entity_type)
    if entity_id is not None:
        info.entity_id = entity_id
    if entity_content is not None:
        info.infos = GameGlobal.proto_encoder.encode(entity_content)
    GameGlobal.gamemgr_proxy.create_entity(info)

def call_global_client_method(method, parameters, reliable=True):
    msg = _C_PB2.GlobalEntityMessage()
    _codec_encoder.encode(msg.method, method)
    msg.parameters = GameGlobal.proto_encoder.encode(parameters)
    msg.reliable = reliable
    GameGlobal.gamemgr_proxy.global_entity_message(msg)

def call_global_group_client_method(target, method, parameters, reliable=True):
    msg = _C_PB2.GlobalEntityMessage()
    msg.target = GameGlobal.proto_encoder.encode(target)
    _codec_encoder.encode(msg.method, method)
    msg.parameters = GameGlobal.proto_encoder.encode(parameters)
    msg.reliable = reliable
    GameGlobal.gamemgr_proxy.global_entity_message(msg)

def call_local_group_client_method(target, method, parameters, reliable=True):
    """调用客户端的RPC"""
    client_list = {'target': []}

    gate_proxies = GameGlobal.game.game_client_mgr.gate_proxy_by_uuid
    for gate_uuid, stub in gate_proxies.items():
        count = 0
        for entity_id in target:
            count += 1

            while True:
                entity = EntityManager.get_entity(entity_id)
                if (entity is None or entity.client is None or
                        entity.client.client_info is None):
                    break
                if gate_uuid == entity.client.client_info.gate_id:
                    client_list['target'].append({
                        'entity_id': entity.entity_id,
                        'session_id': entity.client.client_info.session_id,
                        })
                    break

            if len(client_list['target']) < 100 and count != len(target):
                continue

            msg = _C_PB2.GlobalEntityMessage()
            msg.target = GameGlobal.proto_encoder.encode(client_list)
            _codec_encoder.encode(msg.method, method)
            msg.parameters = GameGlobal.proto_encoder.encode(parameters)
            msg.reliable = reliable
            stub.global_entity_message(msg)

            client_list['target'] = []

def create_entity_locally(entity_type, entity_id=None, entity_content=None):
    """在本地Game Server上创建Entity"""
    entity = EntityFactory.get_instance().create_entity(entity_type, entity_id)
    if entity is not None and entity_content is not None:
        entity.init_from_dict(entity_content)
    return entity

def create_entity_remotely(entity_type, server_info=None,
        entity_id=None, entity_content=None, fromdb=False, callback=None):
    """在指定的Game Server上创建Entity"""
    _create_remote_entity(entity_type, server_info=server_info,
            entity_id=entity_id, entity_content=entity_content,
            fromdb=fromdb, callback=callback)

def create_entity_anywhere(entity_type,
        entity_id=None, entity_content=None, fromdb=False, callback=None):
    """在任意Game Server上创建Entity"""
    _create_remote_entity(entity_type, entity_id=entity_id,
            entity_content=entity_content, fromdb=fromdb, callback=callback)
