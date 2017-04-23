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

import sys
sys.path.append('../External')
sys.path.append('../Core')
from Log.LogManager import LogManager

_logger = LogManager.get_logger('NyxEngine.TestUtils')

class EchoChannelObj(object):
    def __init__(self, conn):
        super(EchoChannelObj, self).__init__()
        self.conn = conn

    def on_disconnected(self):
        _logger.debug('EchoChannelObj.on_disconnected - connection closed')

    def on_input_data(self, data):
        _logger.debug('EchoChannelObj.on_input_data - data(%s)', data)
        self.conn.write_data(data)
        return 2

class LogChannelObj(object):
    def __init__(self, conn):
        super(LogChannelObj, self).__init__()
        self.conn = conn

    def on_disconnected(self):
        _logger.debug('LogChannelObj.on_disconnected - connection closed')

    def on_input_data(self, data):
        _logger.debug('LogChannelObj.on_input_data - data(%s)', data)
        return 2

class ConnectionMgr(object):
    def __init__(self, channel_creator=None):
        super(ConnectionMgr, self).__init__()
        self.channel_creator = channel_creator

    def on_new_connection(self, conn):
        _logger.debug(
                'ConnectionMgr.on_new_connection - new connection(%s)',
                conn.socket.getpeername())
        ch = self.channel_creator(conn)
        conn.set_channel(ch)
