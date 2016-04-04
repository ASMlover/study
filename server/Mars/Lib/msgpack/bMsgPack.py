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

# bson msgpack

import datetime
import os
from bson.objectid import ObjectId

if os.environ.get('MSGPACK_CPP_IMPLEMENTATION'):
    from msgpack import cmsgpack
    ExtType = cmsgpack.ExtType
else:
    import msgpack
    ExtType = msgpack.ExtType

def msgPackExt(obj):
    if isinstance(obj, ObjectId):
        return ExtType(42, str(obj))
    elif isinstance(obj, datetime.datetime):
        return ExtType(43, obj.strftime('%Y-%m-%d-%H-%M-%S-%f'))
    return repr(obj)

def extHook(code, data):
    if code == 42:
        return ObjectId(str(data))
    elif code == 43:
        dataList = [int(v) for v in str(data).split('-')]
        return datetime.datetime(dataList[0], dataList[1], dataList[2],
                dataList[3], dataList[4], dataList[5], dataList[6])
    return ExtType(code, data)


