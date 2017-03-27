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

from common.nyx_codec import CodecEncoder
from common.nyx_id import IdCreator
from log.nyx_log import LogManager
from proto.pyproto.nyx_commom_pb2 import ConnectResponse, EntityInfo, EntityRpcMessage
from proto.pyproto import nyx_gate_game_pb2 as _gg_pb2
from distributed.game import nyx_game_global as _gglobal

class BaseClientProxy(object):
    def __init__(self, stub, client_info=None):
        self.logger = LogManager.get_logger(self.__class__.__name__)
        self.stub = stub
        self.client_info = client_info
        self.cached_client_info_bytes = ''
        self.recv_client_message_seq = None

    def set_stub(self, stub):
        self.stub = stub
        if stub is None:
            return
        stub._rpc_channel.register_listener(self)

    def set_client_info(self, info):
        self.client_info = info
        if info:
            self.cached_client_info_bytes = info.SerializeToString()
        else:
            self.cached_client_info_bytes = ''

    def get_client_addr(self):
        if not self.client_info:
            return None
        return self.client_info.addr

    def set_received_client_message_seq(self, seq):
        self.recv_client_message_seq = seq

    def get_received_client_message_seq(self):
        return self.recv_client_message_seq

    def create_entity(self, entity_type, entity_data, entity_id):
        raise NotImplemented

    def call_client_method(self, method, parameters, entity_id=None, reliable=True):
        raise NotImplemented

    def call_client_method_raw(self, method, parameters, entity_id=None):
        raise NotImplemented

    def connect_response(self, response_type, entity_id=None, extra_msg=None):
        raise NotImplemented

    def become_player(self, entity_id):
        self.call_client_method('become_player', {}, entity_id)

    def notify_destroyed_entity(self, entity_id):
        raise NotImplemented

    def chat_to_client(self, outband_info):
        raise NotImplemented

    def on_transfer_client_callback(self, return_value):
        raise NotImplemented

    def bind_client_to_game(self, client_bindmsg):
        raise NotImplemented

    def send_reg_md5index(self, md5_index):
        """发送md5或index信息到客户端"""
        raise NotImplemented

    def reg_md5index(self, md5_index):
        """处理客户端的注册请求"""
        self.encoder.add_index(md5_index.md5, md5_index.index)

    def set_owner(self, owner):
        """attach该proxy的服务器entity"""
        self.owner = owner

    def get_owner_id(self):
        if self.owner:
            return self.owner.id
        return None

    def on_channel_disconnected(self, rpc_channel):
        """断开连接时回调"""
        if self.owner:
            self.owner.on_lose_client()

    def destroy(self):
        self.clean_stub()

    def is_cachable(self):
        """是否有消息cache"""
        return False

    def is_disconnected(self):
        return self.stub is None

    def clean_stub(self):
        """清理连接"""
        if self.stub:
            self.stub._rpc_channel.unregister_listener(self)
        self.set_stub(None)

    def give_cache_to(self, other):
        raise NotImplemented

    def bind_soul(self, avatar_mailbox, soul_mailbox, callback=None):
        raise NotImplemented

    def unbind_soul(self, callback=None):
        raise NotImplemented

class ClientProxy(BaseClientProxy):
    """客户端代理，用于调用客户端的avatar entity方法"""
    def __init__(self, stub, client_info=None):
        super(ClientProxy, self).__init__(stub, client_info)
        self.owner = None
        self.encoder = CodecEncoder()
        self.set_stub(stub)
        self.set_client_info(client_info)
        self.transfer_entity_callback = None

    def create_entity(self, entity_type, entity_data, entity_id):
        """调用客户端的create_entity方法创建客户端entity"""
        if entity_id is None:
            self.logger.error('ClientProxy.create_entity: need pass entity id')
            return
        entity_info = EntityInfo()
        EntityInfo.routes = self.cached_client_info_bytes
        entity_info.entity_id = entity_id
        self.encoder.encode(entity_info.type, entity_type)
        entity_info.info = _gglobal.proto_encoder.encode(entity_data)
        self.stub.create_entity(None, entity_info)

    def __getattr__(self, name):
        def _caller(*args):
            entity_id = self.get_owner_id()
            msg = EntityRpcMessage()
            msg.routes = self.cached_client_info_bytes
            msg.entity_id = entity_id
            self.encoder.encode(msg.method, name)
            msg.params = _gglobal.proto_encoder.encode({'_': args})
            self.stub.entity_message(None, msg)

        return _caller

    def call_client_method(self, method, parameters, entity_id=None, reliable=True):
        """调用客户端的rpc方法"""
        if entity_id is None:
            entity_id = self.get_owner_id()
            if entity_id is None:
                self.logger.error('ClientProxy.call_client_method: need pass entity id')
                return

        msg = EntityRpcMessage()
        msg.routes = self.cached_client_info_bytes
        msg.entity_id = entity_id
        self.encoder.encode(msg.method, method)
        msg.params = _gglobal.proto_encoder.encode(parameters)
        msg.reliable = reliable
        self.stub.entity_message(None, msg)

    def call_client_method_raw(self, method, parameters, entity_id=None, reliable=True):
        if entity_id is None:
            entity_id = self.get_owner_id()
            if entity_id is None:
                self.logger.error('ClientProxy.call_client_method_raw: need pass entity id')
                return

        msg = EntityRpcMessage()
        msg.routes = self.cached_client_info_bytes
        msg.entity_id = entity_id
        self.encoder.encode(msg.method, method)
        msg.params = parameters
        msg.reliable = reliable
        self.stub.entity_message(None, msg)

    def call_gate_method(self, method, parameters=''):
        if not method:
            return

        msg = _gg_pb2.GateRpcMessage()
        msg.method.md5 = method
        if isinstance(parameters, str):
            msg.params = parameters
        else:
            msg.params = _gglobal.proto_encoder.encode(parameters)
        self.stub.gate_method(None, msg)

    def connect_response(self, response_type, entity_id=None, extra_msg=None):
        response = ConnectResponse()
        response.type = response_type
        if entity_id is not None:
            response.entity_id = entity_id
        if extra_msg:
            response.extra_msg = _gglobal.proto_encoder.encode(extra_msg)
        response.routes = self.cached_client_info_bytes
        self.stub.on_connect_server(None, response)

    def notify_destroyed_entity(self, entity_id):
        """通知客户端entity_id指定的服务器entity对象销毁了"""
        info = EntityInfo()
        info.entity_id = entity_id
        info.routes = self.cached_client_info_bytes
        self.stub.destroy_entity(None, info)

    def notify_disconnect_client(self):
        info = _gg_pb2.ClientInfo()
        info.ParseFromString(self.cached_client_info_bytes)
        self.stub.disconnect_client(None, info)

    def chat_to_client(self, outband_info):
        outband_info.routes = self.cached_client_info_bytes
        self.stub.chat_to_client(None, outband_info)

    def transfer_client(self, client_bindmsg, callback):
        self.transfer_entity_callback = callback
        self.stub.transfer_client(None, client_bindmsg)

    def on_transfer_client_callback(self, return_value):
        if self.transfer_entity_callback:
            self.transfer_entity_callback(return_value.return_value)
            self.transfer_entity_callback = None

    def bind_client_to_game(self, client_bindmsg):
        self.stub.bind_client_to_game(None, client_bindmsg)

    def send_reg_md5index(self, md5_index):
        self.stub.reg_md5index(None, md5_index)

    def reg_md5index(self, md5_index):
        self.encoder.add_index(md5_index.md5, md5_index.index)

    def destroy(self):
        super(ClientProxy, self).destroy()
        self.stub = None
        self.owner = None
        self.logger = None
        self.encoder = None
