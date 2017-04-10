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

from struct import unpack

class Request(object):
    """保存请求的数据，以便异步解析数据"""
    def __init__(self):
        super(Request, self).__init__()
        self.size = ''
        self.data = ''

    def reset(self):
        self.size = ''
        self.data = ''

    def get_size(self):
        try:
            return unpack('<I', self.size)[0]
        except:
            return -1

class RequestParser(object):
    """解析请求的数据，需要记录解析操作的状态"""
    _STATUS_SIZE = 0
    _STATUS_DATA = 1
    _SIZE_BYTES = 4
    _MAX_DATABYTES = 0xFFFFFF

    def __init__(self):
        super(RequestParser, self).__init__()
        self.max_databytes = RequestParser._MAX_DATABYTES
        self.need_bytes = RequestParser._SIZE_BYTES
        self.status = RequestParser._STATUS_SIZE

    def reset(self):
        self.need_bytes = RequestParser._SIZE_BYTES
        self.status = RequestParser._STATUS_SIZE

    def set_max_databytes(self, data_bytes):
        self.max_databytes = data_bytes

    def parse(self, request, data, skip=0):
        l = len(data) - skip

        if self.status == RequestParser._STATUS_SIZE:
            if l < self.need_bytes:
                request.size += data[skip:]
                self.need_bytes -= l
                return 2, l
            else:
                request.size += data[skip : skip + self.need_bytes]
                consum = self.need_bytes
                self.status = RequestParser._STATUS_DATA
                datalen = request.get_size()
                if datalen < 1 or datalen > self.max_databytes:
                    return 0, consum
                self.need_bytes = datalen
                return 2, consum
        elif self.status == RequestParser._STATUS_DATA:
            if self.need_bytes > l:
                request.data += data[skip:]
                self.need_bytes -= l
                return 2, l
            else:
                request.data += data[skip : skip + self.need_bytes]
                consum = self.need_bytes
                self.reset()
                return 1, consum
        else:
            return 0, 0
