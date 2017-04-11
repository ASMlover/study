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

from struct import pack, unpack
from google.protobuf import service
from Log.LogManager import LogManager
from Rpc import Request

class RpcController(service.RpcController):
    def __init__(self, channel):
        super(RpcController, self).__init__()
        self.rpc_channel = channel

class RpcChannel(service.RpcChannel):
    def __init__(self, rpc_service, conn):
        super(RpcChannel, self).__init__()
        self.logger = LogManager.get_logger('Rpc.RpcChannel')

        self.rpc_service = rpc_service
        self.rpc_request = Request.Request()
        self.rpc_request_parser = Request.RequestParser()
        self.conn = conn
        self.conn.set_channel(self)
        self.controller = RpcController(self)
        self.conn_listeners = set()

        # user data information
        self.user_data = None
        self.encrypted = False
        self.compressed = False
        self.session_seed = None

    def set_max_databytes(self, data_bytes):
        self.rpc_request_parser.set_max_databytes(data_bytes)

    def reg_listener(self, listener):
        self.conn_listeners.add(listener)

    def unreg_listener(self, listener):
        self.conn_listeners.discard(listener)

    def get_peeraddr(self):
        if self.conn:
            return self.conn.get_peeraddr()
        return 'No connection attached'

    def sef_compressor(self, compressor):
        self.conn.set_compressor(compressor)

    def set_crypter(self, crypter):
        self.conn.set_crypter(crypter)

    def set_user_data(self, user_data):
        self.user_data = user_data

    def get_user_data(self):
        return self.user_data

    def set_session_seed(self, seed):
        self.session_seed = seed

    def get_session_seed(self):
        return self.session_seed

    def disconnect(self):
        """断开连接"""
        if self.conn:
            self.conn.disconnect()

    def on_disconnected(self):
        """连接(TcpSession)断开时回调"""
        for listener in list(self.conn_listeners):
            # 这里先拷贝到list是为了放置其他线程有使用unreg_*的情况
            if listener in self.conn_listeners:
                listener.on_channel_disconnected(self)
        self.rpc_request.reset()
        self.rpc_request_parser.reset()
        self.conn_listeners = None
        self.conn = None
        self.user_data = None

    def CallMethod(self, method, controller, request, response, done):
        """protobuf RpcChannel的重载，发送rpc"""
        index = method.index
        assert index < 65535
        data = request.SerializeToString()
        datalen = len(data) + 2
        self.conn.write_data(
                ''.join([pack('<I', datalen), pack('<H', index), data]))

    def on_request(self):
        # TODO:
        pass

    def on_input_data(self, data):
        # TODO:
        pass
