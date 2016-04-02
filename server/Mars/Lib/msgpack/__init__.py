# coding: utf-8
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