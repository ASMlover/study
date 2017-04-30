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

import async_time as _at
from Log.LogManager import LogManager
from Rpc.RpcChannel import RpcChannel
from Rpc.TcpClient import TcpClient

class ChannelClient(object):
    _STATUS_INIT = 0
    _STATUS_CONNECTING = 1
    _STATUS_CONNECT_SUCCESSED = 2
    _STATUS_CONNECT_FAILED = 3

    def __init__(self, ip, port, rpc_service):
        super(ChannelClient, self).__init__()
        self.logger = LogManager.get_logger('Rpc.ChannelClient')

        self.rpc_service = rpc_service
        self.ip = ip
        self.port = port
        self.timer = None
        self.client = TcpClient(self.ip, self.port, self)
        self.status = ChannelClient._STATUS_INIT

    def get_peeraddr(self):
        return self.client.get_peeraddr()

    def reset(self, ip=None, port=None):
        self.status = ChannelClient._STATUS_INIT
        if self.client:
            self.client.close()

        if ip:
            self.ip = ip
        if port:
            self.port = port
        self.client = TcpClient(self.ip, self.port, self)
        self.cancel_timer()

    def cancel_timer(self):
        if self.timer and not self.timer.cancelled and not self.timer.expired:
            self.timer.cancel()
            self.timer = None

    def check_connection(self):
        if self.status != ChannelClient._STATUS_CONNECT_SUCCESSED:
            self.logger.error('ChannelClient.check_connection - timeout')
            self.connect_callback and self.connect_callback(None)
            self.status = ChannelClient._STATUS_CONNECT_FAILED

    def connect(self, timeout, callback):
        self.client.async_connect()
        self.connect_callback = callback
        self.status = ChannelClient._STATUS_CONNECTING
        self.timer = _at.add_timer(timeout, self.check_connection)

    def disconnect(self):
        if self.client:
            self.client.disconnect()

    def on_new_connection(self, conn):
        """服务端或客户端有新连接的时候回调"""
        rpc_channel = RpcChannel(self.rpc_service, conn)
        self.status = ChannelClient._STATUS_CONNECT_SUCCESSED
        self.connect_callback(rpc_channel)

    def on_connection_closed(self, conn):
        self.cancel_timer()
        self.status = ChannelClient._STATUS_CONNECT_FAILED
        self.connect_callback(None)
