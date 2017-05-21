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

from Common.Codec import CodecEncoder
from Common.IdUtils import IdUtils
from Distributed.Game import GameGlobal
from Log.LogManager import LogManager
from Proto.Details import BasicProtocol_pb2 as _B_PB2
from Proto.Details.Common_pb2 import ConnectResponse, EntityInfo, EntityMessage

class BaseClientProxy(object):
    """基础的与客户端连接的代理基类"""
    def __init__(self, stub, client_info=None):
        self.logger = LogManager.get_logger(self.__class__.__name__)
        self.owner = None
        self.stub = None
        self.client_info = None
        self.cached_client_infos = '' # 缓存的客户端信息
        self.recved_client_message_seq = None # 客户消息的seq

    def set_stub(self, stub):
        self.stub = stub
        if stub is None:
            return
        stub.rpc_channel.reg_listener(self)

    def set_client_info(self, client_info):
        self.client_info = client_info
        if client_info:
            self.cached_client_infos = client_info.SerializeToString()
        else:
            self.cached_client_infos = ''

    def set_received_client_message_seq(self, seq):
        self.recved_client_message_seq = seq

    def get_received_client_message_seq(self):
        return self.recved_client_message_seq

    def set_owner(self, owner):
        """设置绑定该Proxy的服务器Entity"""
        self.owner = owner

    def get_ownerid(self):
        """获取绑定该Proxy的服务器Entity的ID"""
        if self.owner:
            return self.owner.entity_id
        return None

    def clean_stub(self):
        """清理连接"""
        if self.stub:
            self.stub.rpc_channel.unreg_listener(self)
        self.stub = None

    def destroy(self):
        """销毁该Proxy"""
        self.clean_stub()

    def is_cachable(self):
        """是否使用消息缓存"""
        return False

    def is_disconnected(self):
        """是否断开连接"""
        return self.stub is None

    def on_channel_disconnected(self, rpc_channel):
        """连接断开的时候回调"""
        if self.owner:
            self.owner.on_lose_client()

    def create_entity(self, entity_type, entity_info, entity_id):
        raise NotImplemented

    def call_client_method(self,
            method, parameters, entity_id=None, reliable=True):
        raise NotImplemented

    def call_client_method_raw(self, method, parameters, entity_id=None):
        raise NotImplemented

    def connect_response(self, response_type, entity_id=None, extra_msg=None):
        raise NotImplemented

    def become_player(self, entity_id):
        self.call_client_method('become_player', {}, entity_id)

    def notify_entity_destroyed(self, entity_id):
        raise NotImplemented

    def chat_to_client(self, outband_info):
        raise NotImplemented

    def transfer_client_callback(self, return_value):
        raise NotImplemented

    def bind_client_to_game(self, bind_msg):
        raise NotImplemented

    def send_reg_md5index(self, md5_index):
        """发送注册的md5_index到客户端"""
        raise NotImplemented

    def give_cache_to(self, other):
        """转移缓存"""
        raise NotImplemented

class ClientProxy(BaseClientProxy):
    """服务端Avatar的客户端代理，用于调用客户度Avatar的Entity方法"""
    def __init__(self, stub, client_info=None):
        super(ClientProxy, self).__init__(stub, client_info)
        self.encoder = CodecEncoder()
        self.set_stub(stub)
        self.set_client_info(client_info)
        self.transfer_entity_callback = None

    def create_entity(self, entity_type, entity_info, entity_id):
        """调用客户端的create_entity方法创建客户端Entity"""
        if entity_id is None:
            self.logger.error('ClientProxy.create_entity: need entity id')
            return
        info = EntityInfo()
        info.routes = self.cached_client_infos
        info.entity_id = entity_id
        self.encoder.encode(info.type, entity_type)
        info.infos = GameGlobal.proto_encoder.encode(entity_info)
        self.stub.create_entity(None, info)

    def __getattr__(self, name):
        def _caller(*args):
            msg = EntityMessage()
            msg.routes = self.cached_client_infos
            msg.entity_id = self.get_ownerid()
            self.encoder.encode(msg.method, name)
            msg.parameters = GameGlobal.proto_encoder.encode({'_': args})
            self.stub.entity_message(None, msg)
        return _caller

    def destroy(self):
        """销毁自己"""
        super(ClientProxy, self).destroy()
        self.logger = None
        self.stub = None
        self.owner = None
        self.encoder = None

    def call_client_method(self,
            method, parameters, entity_id=None, reliable=True):
        """调用客户端的RPC方法

        Args：
            method - 字符串，表示调用的方法名
            parameters - 一个dict，表示调用的参数
            entity_id - 默认使用owner的entity_id
        """
        if entity_id is None:
            entity_id = self.get_ownerid()
            if entity_id is None:
                self.logger.error(
                        'ClientProxy.call_client_method: need entity id')
                return
        msg = EntityMessage()
        msg.routes = self.cached_client_infos
        msg.entity_id = entity_id
        self.encoder.encode(msg.method, method)
        msg.parameters = GameGlobal.proto_encoder.encode(parameters)
        msg.reliable = reliable
        self.stub.entity_message(None, msg)

    def call_client_method_raw(self,
            method, parameters, entity_id=None, reliable=True):
        """原始的调用客户端方法，参数无压缩"""
        if entity_id is None:
            entity_id = self.get_ownerid()
            if entity_id is None:
                self.logger.error(
                        'ClientProxy.call_client_method_raw: need entity id')
                return
        msg = EntityMessage()
        msg.routes = self.cached_client_infos
        msg.entity_id = entity_id
        msg.encoder.encode(msg.method, method)
        msg.parameters = parameters
        msg.reliable = reliable
        self.stub.entity_message(None, msg)

    def call_gate_method(self, method, parameters=''):
        """调用Gate的RPC方法"""
        if not method:
            return
        msg = _B_PB2.GateMessage()
        msg.method.md5 = method
        if isinstance(parameters, str):
            msg.parameters = parameters
        else:
            msg.parameters = GameGlobal.proto_encoder.encode(parameters)
        self.stub.gate_method(None, msg)

    def connect_response(self, response_type, entity_id=None, extra_msg=None):
        """客户端连接的回复"""
        response = ConnectResponse()
        response.routes = self.cached_client_infos
        response.type = response_type
        if entity_id is not None:
            response.entity_id = entity_id
        if extra_msg:
            response.extra_msg = GameGlobal.proto_encoder.encode(extra_msg)
        self.stub.connect_response(None, response)

    def notify_entity_destroyed(self, entity_id):
        """通知客户端某个客户端的Entity已经销毁了"""
        info = EntityInfo()
        info.routes = self.cached_client_infos
        info.entity_id = entity_id
        self.stub.destroy_entity(None, info)

    def notify_disconnect_client(self):
        """通知断开客户端的连接"""
        info = _B_PB2.ClientInfo()
        info.ParseFromString(self.cached_client_infos)
        if self.stub:
            self.stub.disconnect_client(None, info)

    def chat_to_client(self, outband_info):
        outband_info.routes = self.cached_client_infos
        self.stub.chat_to_client(None, outband_info)

    def transfer_client(self, bind_msg, callback):
        """转移客户端Entity"""
        self.stub.transfer_client(None, bind_msg)
        self.transfer_entity_callback = callback

    def transfer_client_callback(self, return_value):
        if self.transfer_entity_callback:
            self.transfer_entity_callback(return_value.return_status)
            self.transfer_entity_callback = None

    def bind_client_to_game(self, bind_msg):
        self.stub.bind_client_to_game(None, bind_msg)

    def send_reg_md5index(self, md5_index):
        self.stub.reg_md5index(None, md5_index)

    def reg_md5index(self, md5_index):
        """处理客户端的注册请求"""
        self.encoder.add_index(md5_index.md5, md5_index.index)
