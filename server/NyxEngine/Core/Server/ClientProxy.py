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
