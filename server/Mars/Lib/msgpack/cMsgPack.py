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

version = (0, 1)

import struct
if struct.calcsize('P') == 4:
    import _cmsgpack
else:
    from x64 import _cmsgpack

ExtType = _cmsgpack.ExtType

class PackException(Exception):
    """Base class for exceptions encountered during packing."""
    pass

class UnpackException(Exception):
    """Base class for exceptions encountered during unpacking."""
    pass

# Packing exceptions
class UnsupportedTypeException(PackException):
    """Object type not supported for packing."""
    pass

# Unpacking exceptions
class InsufficientDataException(UnpackException):
    """Insufficient data to unpack the encoded object."""
    pass

class InvalidStringException(UnpackException):
    """Invalid UTF-8 string encountered during unpacking."""
    pass

class ReservedCodeException(UnpackException):
    """Reserved code encountered during unpacking."""
    pass

class UnhashableKeyException(UnpackException):
    """Unhashable key encountered during map unpacking.
    The serialized map can not be deserialized into a Python dict.
    """
    pass

class DuplicateKeyException(UnpackException):
    """Duplicate key encountered during map packing."""
    pass

KeyNotPrimitiveException = UnhashableKeyException
KetDuplicateException = DuplicateKeyException

pack = None
packb = None
unpack = None
unpackb = None
dump = None
dumps = None
load = None
loads = None

def pack(obj, default=None):
    return _cmsgpack._pack(obj, default)

def unpack(obj, extHook=None):
    return _cmsgpack._unpack(obj, extHook)
