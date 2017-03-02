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

import async_timer as _at
from log.nyx_log import LogManager
from rpc.nyx_rpc_channel import NyxRpcChannel
from rpc.nyx_tcp_client import TcpClient

class ChannelClient(object):
    _STATUS_INIT = 0
    _STATUS_CONNECTING = 1
    _STATUS_CONNECTFAILED = 2
    _STATUS_CONNECTSUCCESS = 3

    def __init__(self, addr, port, rpc_service):
        super(ChannelClient, self).__init__()
        self._logger = LogManager.get_logger('NyxCore.Rpc.ChannelClient')
        self._rpc_service = rpc_service
        self._addr = addr
        self._port = port
        self._timer = None
        self._client = TcpClient(self._addr, self._port, self)
        self._status = ChannelClient._STATUS_INIT

    def get_peeraddr(self):
        return self._client.get_peeraddr()

    def reset(self, addr=None, port=None):
        # TODO:
        pass

    def cancel_timer(self):
        # TODO:
        pass

    def check_connection(self):
        # TODO:
        pass

    def connect(self, timeout, callback):
        # TODO:
        pass

    def disconnect(self):
        if self._client:
            self._client.disconnect()

    def on_new_connection(self, conn):
        # TODO:
        pass

    def on_connection_failed(self, conn):
        # TODO:
        pass
