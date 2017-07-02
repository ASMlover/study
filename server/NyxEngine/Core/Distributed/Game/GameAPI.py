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

    gate_proxies = GameGlobal.game.game_client_mgr.gate_proxies_by_uuid
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

def create_entity_servertype(entity_type, server_type,
        entity_id=None, entity_content=None, fromdb=False, callback=None):
    """根据指定的Server Type在任意Game Server上创建Entity"""
    if not GameGlobal.gamemgr_proxy.connected:
        _logger.error(
                'create_entity_servertype: lost connection with game manager')
        return

    server_info = _C_PB2.ServerInfo()
    server_info.server_type = server_type

    header = _B_PB2.EntityInfoHeader()
    header.dst_server.CopyFrom(server_info)
    header.create_anywhere = _B_PB2.EntityInfoHeader.SPECIFY_SERVER
    header.transfer_entity = False
    header.create_fromdb = fromdb
    if callback is not None:
        header.callback_id = GameGlobal.reg_gamemgr_callback(callback)

    info = _C_PB2.EntityInfo()
    info.routes = header.SerializeToString()
    _codec_encoder.encode(info.type, entity_type)
    if entity_id is not None:
        info.entity_id = entity_id
    if entity_content is not None:
        info.infos = GameGlobal.proto_encoder.encode(entity_content)
    GameGlobal.gamemgr_proxy.create_entity(info)

def register_entity_globally(
        entity_unique_id, entity, callback=None, override=False):
    """将Entity注册到全局，如果用相同的key且override为False则失败"""
    if not GameGlobal.gamemgr_proxy.connected:
        _logger.error(
                'register_entity_globally: lost connection with game manager')
        return

    msg = _B_PB2.GlobalEntityRegMessage()
    msg.entity_unique_id = entity_unique_id
    msg.mailbox.entity_id = entity.entity_id
    msg.mailbox.server_info.CopyFrom(GameGlobal.game_info)
    if callback is not None:
        msg.callback_id = GameGlobal.reg_gamemgr_callback(callback)
    if override:
        msg.override = True
    GameGlobal.gamemgr_proxy.reg_entity_mailbox(msg)

def get_global_entity_mailbox(entity_unique_id):
    return GameGlobal.global_entities.get(entity_unique_id)

def encode_mailbox(mailbox):
    if mailbox is None:
        return None
    return Binary(mailbox.SerializeToString())

def decode_mailbox(mbbytes):
    if mbbytes is None:
        return None
    mailbox = _C_PB2.EntityMailbox()
    mailbox.ParseFromString(mbbytes)
    return mailbox

def is_same_mailbox(mb1, mb2):
    return (mb1.entity_id == mb2.entity_id and
            mb1.server_info.ip == mb2.server_info.ip and
            mb1.server_info.port == mb2.server_info.port)

def get_mailbox_entity_id(mb):
    return mb.entity_id

def is_local_server(server_info):
    local_server = GameGlobal.game_info
    return (local_server.ip == server_info.ip and
            local_server.port == server_info.port)

def get_dbmgr_proxy(entity_id=None):
    def hash_code(bytes):
        if len(GameGlobal.dbmgr_proxy_set) == 1:
            return 0
        key = 0
        for c in bytes:
            key += ord(c)
        return key % len(GameGlobal.dbmgr_proxy_set)

    if entity_id is not None:
        return GameGlobal.dbmgr_proxy_set[hash_code(entity_id)]
    else:
        import random
        return random.choice(GameGlobal.dbmgr_proxy_set)

def _get_dbproxy_wrapper(callback=None, *args):
    dbproxy = get_dbmgr_proxy()
    if dbproxy is None:
        _logger.warn('_get_dbproxy_wrapper: db manager not connected')
        if callback:
            if args:
                callback(None, *args)
            else:
                callback(None)
    return dbproxy

def find(collection, query, fields, callback, read_pref=None, hint=None):
    """调用dbmgr查找满足条件的记录"""
    dbproxy = _get_dbproxy_wrapper(callback)
    if dbproxy is None:
        return False

    def _find_callback(status, docs):
        if status and len(docs) == 1:
            callback(docs[0])
        else:
            callback(None)
    dbproxy.db_find_doc(GameGlobal.dbname, collection,
            query, fields, 1, _find_callback, read_pref=read_pref, hint=hint)
    return True

def find_multi(collection,
        query, fields, callback, read_pref=None, hint=None, limit=1):
    """查找满足条件的多条记录"""
    dbproxy = _get_dbproxy_wrapper(callback)
    if dbproxy is None:
        return False

    def _find_multi_callback(status, docs):
        if status:
            callback(docs)
        else:
            callback(None)
    dbproxy.db_find_doc(GameGlobal.dbname, collection, query, fields,
            limit, _find_multi_callback, read_pref=read_pref, hint=hint)
    return True

def find_data_record(collection, record_id, callback, read_pref=None):
    """查找指定的记录，如果存在则callback返回"""
    dbproxy = _get_dbproxy_wrapper(callback)
    if dbproxy is None:
        return False

    def _find_record_callback(status, docs):
        if status and len(docs) == 1:
            callback(docs[0])
        else:
            callback(None)
    dbproxy.db_find_doc(GameGlobal.dbname, collection, {'_id': record_id},
            None, 1, _find_record_callback, read_pref=read_pref)
    return True

def find_and_modify(collection,
        query, update, fields, upsert, new, callback, seq_flag=False):
    dbproxy = _get_dbproxy_wrapper(callback)
    if dbproxy is None:
        return False

    dbproxy.db_find_and_modify_doc(GameGlobal.dbname, collection,
            query, update, fields, upsert, new, callback, seq_flag)
    return True

def update(collection, query, fields, upsert=True, multi=False, callback=None):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_update_doc(GameGlobal.dbname,
            collection, query, fields, callback, upsert, multi)
    return True

def update_data_record(
        collection, record_id, data, callback=None, upsert=True):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_update_doc(GameGlobal.dbname, collection,
            {'_id': record_id}, {'$set': data}, callback, upsert)
    return True

def insert_data_record(collection, record_id=None, data={}, callback=None):
    """插入一条记录"""
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    if not record_id:
        insert_data = {}
    else:
        insert_data = {'_id': record_id}
    insert_data.update(data)
    dbproxy.db_insert_doc(GameGlobal.dbname, collection, insert_data, callback)
    return True

def delete_data_record(collection, record_id, callback=None):
    """删除一条记录"""
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_delete_doc(GameGlobal.dbname,
            collection, {'_id': record_id}, callback)
    return True

def ensure_db_index(dbname, collection, index, desc=None, callback=None):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_operation_index(dbname, collection,
            _B_PB2.OperationIndexRequest.ENSURE, index, desc, callback)
    return True

def drop_db_index(dbname, collection, callback=None):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_operation_index(dbname, collection,
            _B_PB2.OperationIndexRequest.DROP, None, None, callback)
    return True

def reset_db_index(dbname, collection, index, desc=None, callback=None):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_operation_index(dbname, collection,
            _B_PB2.OperationIndexRequest.RESET, index, desc, callback)
    return True

def create_collection(dbname, collection, options={}, callback=None):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_create_collection(dbname, collection, options, callback)
    return True

def _on_game_traceback():
    """traceback时调用"""
    import sys
    t, v, tb = sys.exc_info()
    GameGlobal.game_event_callback.on_traceback(t, v, tb)

def create_entity_fromdb(entity_type, entity_id, callback=None):
    """从DB中恢复数据并创建Entity"""
    dbproxy = _get_dbproxy_wrapper(callback)
    if dbproxy is None:
        return False

    EntityClass = EntityFactory.get_instance().get_entity_class(entity_type)
    if not EntityClass:
        _logger.error('create_entity_fromdb: entity class not registered')
        callback(None)
        return False

    def _load_entity_callback(status, docs):
        if status and len(docs) == 1:
            entity = EntityClass(entity_id)
            try:
                entity.init_from_dict(docs[0])
            except:
                _logger.error('create_entity_fromdb: init_from_dict error')
                _on_game_traceback()
                _logger._log_exception()
                entity.cancel_save_timer()
                EntityManager.del_entity(entity_id)
                callback(None)
            else:
                callback(entity)
        else:
            callback(None)
    fields = getattr(EntityClass.init_from_dict, 'fieldlist', None)
    dbproxy.db_find_doc(GameGlobal.dbname, 'entities',
            {'_id': entity_id}, fields, 1, _load_entity_callback, seq_flag=True)
    return True

def load_entity_dict_fromdb(entity_type, entity_id, callback):
    """从DB中恢复Entity的数据信息"""
    dbproxy = _get_dbproxy_wrapper(callback, entity_id)
    if dbproxy is None:
        return False

    EntityClass = EntityFactory.get_instance().get_entity_class(entity_type)
    if not EntityClass:
        _logger.error('load_entity_dict_fromdb: entity claass not registered')
        return False

    def _load_entity_dict_callback(status, docs):
        if status and len(docs) == 1:
            callback(docs[0], entity_id)
        else:
            callback(None, entity_id)
    fields = getattr(EntityClass.init_from_dict, 'fieldlist', None)
    dbproxy.db_find_doc(GameGlobal.dbname, 'entities',
            {'_id': entity_id}, fields, 1, _load_entity_dict_callback)
    return True

def del_entity(entity_id, callback=None):
    dbproxy = _get_dbproxy_wrapper()
    if dbproxy is None:
        return False

    dbproxy.db_delete_doc(GameGlobal.dbname,
            'entities', {'_id': entity_id}, callback)
    return True

def save_entity(entity, callback=None):
    entity_id = entity.entity_id
    dbproxy = get_dbmgr_proxy(entity_id)
    if dbproxy is None:
        _logger.warn('save_entity: db manager not connected')
        return False

    update_dict = {'entity_class': entity.__class__.__name__}
    try:
        save_dict = entity.get_persistent_dict()
    except:
        if callback is not None:
            callback(False)
        _logger.error('save_entity: get_persistent_dict for entity failed')
        _logger._log_exception()
        return False
    update_dict.update(save_dict)
    dbproxy.db_update_doc(GameGlobal.dbname, 'entities',
            {'_id': entity_id}, {'$set': update_dict}, callback, seq_flag=True)
    return True
