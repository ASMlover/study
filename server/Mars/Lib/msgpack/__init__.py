#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
from msgpack._version import version
from msgpack.exceptions import *
from collections import namedtuple

class ExtType(namedtuple('ExtType', 'code data')):
    """ExtType represents ext type in msgpack."""
    def __new__(cls, code, data):
        if not isinstance(code, int):
            raise TypeError("code must be int")
        if not isinstance(data, bytes):
            raise TypeError("data must be bytes")
        if not 0 <= code <= 127:
            raise ValueError("code must be 0~127")
        return super(ExtType, cls).__new__(cls, code, data)

import os
if os.environ.get('MSGPACK_CPP_IMPLEMENTATION'):
    try:
        import msgpack.cmsgpack as cmsgpack
    except ImportError:
        from msgpack.fallback import Packer, unpackb, Unpacker
else:
    from msgpack.fallback import Packer, Unpacker


def packb(o, **kwargs):
    """
    Pack object `o` and return packed bytes
    See :class:`Packer` for options.
    """
    if os.environ.get('MSGPACK_CPP_IMPLEMENTATION'):
        return cmsgpack.pack(o, **kwargs)
    else:
        return Packer(**kwargs).pack(o)

def unpackb(packed, **kwargs):
    """
    Unpack an object from `packed`.

    Raises `ExtraData` when `packed` contains extra bytes.
    See :class:`Unpacker` for options.
    """
    if os.environ.get('MSGPACK_CPP_IMPLEMENTATION'):
        ret = cmsgpack.unpack(packed, **kwargs)
    else:
        unpacker = Unpacker(None, **kwargs)
        unpacker.feed(packed)
        try:
            ret = unpacker._fb_unpack()
        except OutOfData:
            raise UnpackValueError("Data is not enough.")
        #if unpacker._fb_got_extradata():
        #    raise ExtraData(ret, unpacker._fb_get_extradata())
    return ret

# alias for compatibility to simplejson/marshal/pickle.
loads = unpackb
dumps = packb
