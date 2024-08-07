#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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
from pyRpc.logger import LoggerMgr
from pyRpc.rpc_controller import RpcController

class RpcParser(object):
    ST_HEAD = 0
    ST_DATA = 1

    def __init__(self, service, head_format, index_format):
        self.logger = LoggerMgr.get_logger('pyRpc.RpcParser')
        self.service = service
        self.head_format = head_format
        self.index_format = index_format
        self.head_bytes = struct.calcsize(self.head_format)
        self.index_bytes = struct.calcsize(self.index_format)

        self.buf = ''
        self.status = RpcParser.ST_HEAD
        self.data_bytes = 0

    def parse(self, data):
        rpc_calls = []
        self.buf += data
        while True:
            if self.status == RpcParser.ST_HEAD:
                self.logger.debug('RpcParser.parse: ST_HEAD: %d/%d', len(self.buf), self.head_bytes)
                if len(self.buf) < self.head_bytes:
                    break

                head_data = self.buf[:self.head_bytes]
                self.data_bytes = struct.unpack(self.head_format, head_data)[0]
                self.buf = self.buf[self.head_bytes:]
                self.status = RpcParser.ST_DATA
            elif self.status == RpcParser.ST_DATA:
                self.logger.debug('RpcParser.parse: ST_DATA: %d/%d', len(self.buf), self.data_bytes)
                if len(self.buf) < self.data_bytes:
                    break

                index_data = self.buf[:self.index_bytes]
                request_data = self.buf[self.index_bytes:self.data_bytes]

                index = struct.unpack(self.index_format, index_data)[0]
                service_desc = self.service.GetDescriptor()

                method = service_desc.methods[index]
                request = self.service.GetRequestClass(method)()

                request.ParseFromString(request_data)
                if not request.IsInitialized():
                    raise AttributeError('invalid request data')

                self.buf = self.buf[self.data_bytes:]
                self.status = RpcParser.ST_HEAD

                rpc_calls.append((method, request))
        return rpc_calls

class RpcChannel(service.RpcChannel):
    HEAD_FORMAT  = '!I'
    HEAD_BYTES   = struct.calcsize(HEAD_FORMAT)
    INDEX_FORMAT = '!H'
    INDEX_BYTES  = struct.calcsize(INDEX_FORMAT)

    def __init__(self, service, conn):
        super(RpcChannel, self).__init__()
        self.logger = LoggerMgr.get_logger('pyRpc.RpcChannel')
        self.service = service
        self.conn = conn

        self.conn.set_rpc_channel(self)
        self.rpc_controller = RpcController(self)
        self.rpc_parser = RpcParser(self.service, RpcParser.HEAD_FORMAT, RpcParser.INDEX_FORMAT)

    def get_peername(self):
        return self.conn and self.conn.get_peername() or (None, None)

    def disconnect(self):
        self.conn and self.conn.disconnect()

    def on_disconnected(self):
        self.conn = None

    def CallMethod(self, method, rpc_controller, request, response_class, done):
        index = method.index
        data = request.SerializeToString()
        data_bytes = RpcChannel.INDEX_BYTES + len(data)

        self.conn.write_data(struct.pack(RpcChannel.HEAD_FORMAT, data_bytes))
        self.conn.write_data(struct.pack(RpcChannel.INDEX_FORMAT, index))
        self.conn.write_data(data)

    def read_data(self, data):
        try:
            rpc_calls = self.rpc_parser.parse(data)
        except (AttributeError, IndexError) as e:
            self.logger.error('RpcChannel.read_data: parse request error, give up and disconnect')
            self.disconnect()
            return

        for method, request in rpc_calls:
            self.service.CallMethod(method, self.rpc_controller, request, callback=None)
