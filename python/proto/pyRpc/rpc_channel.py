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

    def __init__(self, rpc_service, head_format, index_format):
        self.logger = LoggerMgr.get_logger('pyRpc.RpcParser')
        self.service = rpc_service
        self.head_format = head_format
        self.index_format = index_format
        self.head_size = struct.calcsize(self.head_format)
        self.index_size = struct.calcsize(self.index_format)

        self.buf = ''
        self.status = RpcParser.ST_HEAD
        self.data_size = 0

    def parse(self, data):
        rpc_calls = []
        self.buf += data
        while True:
            if self.status == RpcParser.ST_HEAD:
                self.logger.debug('RpcParser.parse: ST_HEAD: %d/%d', len(self.buf), self.head_size)
                if len(self.buf) < self.head_size:
                    break

                head_data = self.buf[:self.head_size]
                self.data_size = struct.unpack(self.head_format, head_data)[0]
                self.buf = self.buf[self.head_size:]
                self.status = RpcParser.ST_DATA
            elif self.status == RpcParser.ST_DATA:
                self.logger.debug('RpcParser.parse: ST_DATA: %d/%d', len(self.buf), self.data_size)
                if len(self.buf) < self.data_size:
                    break

                index_data = self.buf[:self.index_size]
                request_data = self.buf[self.index_size:self.data_size]

                index = struct.unpack(self.index_format, index_data)[0]
                service_desc = self.service.GetDescriptor()

                method = service_desc.methods[index]
                request = self.service.GetRequestClass(method)()

                request.ParseFromString(request_data)
                if not request.IsInitialized():
                    raise AttributeError('invalid request data')

                self.buf = self.buf[self.data_size:]
                self.status = RpcParser.ST_HEAD

                rpc_calls.append((method, request))
        return rpc_calls

class RpcChannel(service.RpcChannel):
    pass
