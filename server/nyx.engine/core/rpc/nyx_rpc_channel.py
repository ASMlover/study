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

import struct
from google.protobuf import service
from log.nyx_log import LogManager
from rpc import nyx_request as _request

class NyxRpcController(service.RpcController):
    def __init__(self, channel):
        super(NyxRpcController, self).__init__()
        self._rpc_chanel = channel

class NyxRpcChannel(service.RpcChannel):
    def __init__(self, rpc_service, conn):
        super(NyxRpcChannel, self).__init__()
        self._logger = LogManager.get_logger('NyxCore.Rpc.NyxRpcChannel')

        self._rpc_service = rpc_service
        self._rpc_request = _request.Request()
        self._rpc_request_parser = _request.RequestParser()
        self._conn = conn
        self._conn.set_channel(self)
        self._controller = NyxRpcController(self)
        self._conn_listeners = set()
        # user data information
        self._user_data = None
        self._encrypted = False
        self._compressed = False
        self._session_seed = None

    def set_max_datalen(self, datalen):
        self._rpc_request_parser.set_max_datalen(datalen)

    def register_listener(self, listener):
        self._conn_listeners.add(listener)
        self._logger.debug('NyxRpcChannel.register_listener - register listener')

    def unregister_listener(self, listener):
        self._conn_listeners.discard(listener)

    def get_peeraddr(self):
        if self._conn:
            return self._conn.get_peeraddr()
        return 'No connection attached'

    def set_compressor(self, compressor):
        self._conn.set_compressor(compressor)

    def set_crypter(self, encrypter, decrypter):
        self._conn.set_crypter(encrypter, decrypter)

    def set_user_data(self, user_data):
        self._user_data = user_data

    def get_user_data(self):
        return self._user_data

    def set_session_seed(self, seed):
        self._session_seed = seed

    def get_session_seed(self):
        return self._session_seed

    def disconnect(self):
        """断开连接"""
        if self._conn:
            self._conn.disconnect()

    def on_disconnected(self):
        """连接(TcpSession)断开时回调"""
        for listener in self._conn_listeners:
            listener.on_channel_disconnected(self)
        self._rpc_request.reset()
        self._rpc_request_parser.reset()
        self._conn_listeners = None
        self._conn = None
        self._user_data = None

    def CallMethod(self, method, controller, request, response, done):
        """protobuf重载，发送rpc需要"""
        index = method.index
        assert index < 65535
        data = request.SerializeToString()
        datalen = len(data) + 2
        self._conn.write_data(''.join([struct.pack('<I', datalen), struct.pack('<H', index), data]))

    def parse(self, data, skip):
        pass

    def request(self, method, request):
        pass

    def input_data(self, data):
        pass
