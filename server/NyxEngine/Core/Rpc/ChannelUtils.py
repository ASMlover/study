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

from Log.LogManager import LogManager
from Rpc.RpcChannel import RpcChannel

class ChannelCreator(object):
    def __init__(self, rpc_service, handler, max_databytes=0):
        super(ChannelCreator, self).__init__()
        self.logger = LogManager.get_logger('Rpc.ChannelCreator')

        self.rpc_service = rpc_service
        self.handler = handler
        self.max_databytes = max_databytes

    def set_max_databytes(self, databytes):
        self.max_databytes = databytes

    def on_new_connection(self, conn):
        """有新连接的时候被服务器或客户端回调"""
        rpc_channel = RpcChannel(self.rpc_service, conn)
        if self.max_databytes > 0:
            rpc_channel.set_max_databytes(self.max_databytes)
        self.handler.on_new_channel(rpc_channel)

    def on_connection_failed(self, conn):
        """连接断开的时候回调"""
        self.logger.info('ChannelCreator.on_connection_failed')
