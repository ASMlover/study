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

from struct import unpack

class Request(object):
    """解析是异步的，用于保存接收到的数据"""

    def __init__(self):
        super(Request, self).__init__()
        self.reset()

    def reset(self):
        self.size = ''
        self.data = ''

    def getSize(self):
        try:
            return unpack('<I', self.size)[0]
        except:
            return -1

class RequestParser(object):
    """解析请求的数据

    请求数据格式是一个头存放长度，后面跟着数据
    """
    SIZE_BYTES = 4
    MAX_DATA_BYTES = 0xFFFFFF

    ST_SIZE = 0
    ST_DATA = 1

    def __init__(self):
        super(RequestParser, self).__init__()
        self.reset()
        self.maxDataBytes = MAX_DATA_BYTES

    def reset(self):
        self.status = RequestParser.ST_SIZE
        self.needBytes = RequestParser.SIZE_BYTES

    def setMaxDataBytes(self, maxBytes):
        self.maxDataBytes = maxBytes

    def parse(self, request, data, skipBytes=0):
        parseBytes = len(data) - skipBytes

        if self.status == RequestParser.ST_SIZE:
            if self.needBytes > parseBytes:
                request.size += data[skipBytes:]
                self.needBytes -= parseBytes
                return 2, parseBytes
            else:
                request.size += data[skipBytes:skipBytes+self.needBytes]
                consumBytes = self.needBytes
                self.status = RequestParser.ST_DATA

                dataBytes = request.getSize()
                if dataBytes < 1:
                    return 0, consumBytes
                if dataBytes > self.maxDataBytes:
                    return 3, consumBytes
                self.needBytes = dataBytes
                return 2, consumBytes
        elif self.status == RequestParser.ST_DATA:
            if self.needBytes > parseBytes:
                request.data += data[skipBytes:]
                self.needBytes -= parseBytes
                return 2, parseBytes
            else:
                request.data += data[skipBytes:skipBytes+self.needBytes]
                consumBytes = self.needBytes

                self.reset()
                return 1, consumBytes
        else:
            return 0, 0
