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

# [doc]
# ChannelListener
#   - on_channel_disconnected(...) # TODO: 需要实现on_channel_disconnected回调

class NyxRpcController(service.RpcController):
    def __init__(self, channel):
        super(NyxRpcController, self).__init__()
        self._rpc_channel = channel

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
        for listener in list(self._conn_listeners):
            # 这里先拷贝到list是为了防止在其他线程有unreg的情况
            if listener in self._conn_listeners:
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

    def on_request(self):
        l = len(self._rpc_request.data)
        if l < 2:
            self._logger.error('NyxRpcChannel.on_request - get error request size: %d', l)
            return False

        index_data = self._rpc_request.data[0:2]
        cmd_index = struct.unpack('<H', index_data)[0]
        rpc_service = self._rpc_service
        descriptor = rpc_service.GetDescriptor()

        if cmd_index > len(descriptor.methods):
            self._logger.error('NyxRpcChannel.on_request - get error method index: %d', cmd_index)
            return False
        method = descriptor.methods[cmd_index]

        try:
            request = rpc_service.GetRequesetClass(method)()
            serilized = self._rpc_request.data[2:]
            request.ParseFromString(serilized)
            rpc_service.CallMethod(method, self._controller, request, None)
        except:
            self._logger.error('NyxRpcChannel.on_request - call rpc method failed')
            self._logger.nyxlog_except()

        return True

    def input_data(self, data):
        total_bytes = len(data)
        skip = 0
        while skip < total_bytes:
            result, consum = self._rpc_request_parser.parse(self._rpc_request, data, skip)
            assert consum > 0

            skip += consum
            if result == 1:
                ok = self.on_request()
                self._rpc_request.reset()
                if not ok:
                    return 0
                continue
            elif result == 0:
                return 0
            else:
                continue

        # 需要更多的数据
        return 2

class RpcChannelCreator(object):
    def __init__(self, rpc_service, handler, max_datalen=0):
        super(RpcChannelCreator, self).__init__()
        self._logger = LogManager.get_logger('NyxCore.Rpc.RpcChannelCreator')
        self._rpc_service = rpc_service
        self._handler = handler
        self._max_datalen = max_datalen

    def set_max_datalen(self, datalen):
        self._max_datalen = datalen

    def on_new_connection(self, conn):
        """TcpServer或TcpClient新连接建立的时候回调"""
        rpc_channel = NyxRpcChannel(self._rpc_service, conn)
        if self._max_datalen > 0:
            rpc_channel.set_max_datalen(self._max_datalen)

        self._handler.on_new_channel(rpc_channel)

    def on_connection_failed(self, conn):
        """连接断开的时候回调"""
        self._logger.info('RpcChannelCreator.on_connection_failed')

class RpcChannelHolder(object):
    def __init__(self):
        super(RpcChannelHolder, self).__init__()
        self._logger = LogManager.get_logger('NyxCore.Rpc.RpcChannelHolder')
        self._rpc_channel = None

    def get_rpc_channel(self):
        return self._rpc_channel

    def on_new_channel(self, rpc_channel):
        """TcpServer或TcpClient处理新连接时的回调"""
        self._rpc_channel = rpc_channel
        rpc_channel.register_listener(self)

    def on_channel_disconnected(self, rpc_channel):
        """连接断开的时候channel回调"""
        self._rpc_channel = None

class RpcChannelManager(object):
    def __init__(self):
        super(RpcChannelManager, self).__init__()
        self._logger = LogManager.get_logger('NyxCore.Rpc.RpcChannelManager')
        self._rpc_channels = {}

    def get_rpc_channel(self, peeraddr):
        return self._rpc_channels.get(peeraddr)

    def get_rpc_channel_count(self):
        return len(self._rpc_channels)

    def on_new_channel(self, rpc_channel):
        """TcpServer或TcpClient在处理新channel建立连接的时候回调"""
        self._rpc_channels[rpc_channel.get_peeraddr()] = rpc_channel
        rpc_channel.register_listener(self)

    def on_channel_disconnected(self, rpc_channel):
        """channel的connection连接断开的时候回调"""
        peeraddr = rpc_channel.get_peeraddr()
        if peeraddr in self._rpc_channels:
            self._rpc_channels.pop(peeraddr, None)
        else:
            self._logger.info('RpcChannelManager.on_channel_disconnected - disconnected connection %s', peeraddr)
