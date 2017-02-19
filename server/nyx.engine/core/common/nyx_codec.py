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

import binascii
import hashlib
from log.nyx_log import LogManager

_NYXCORE_DEBUG = 0
_logger = LogManager.get_logger('NyxCore.Codec')

class Md5Cache(object):
    _str2md5 = {}
    _md52str = {}

    @staticmethod
    def get_md5(string):
        md5 = Md5Cache._str2md5.get(string)
        if md5 is None:
            if _NYXCORE_DEBUG:
                Md5Cache._str2md5[string] = string
                Md5Cache._md52str[string] = string
            else:
                md5_gen = hashlib.md5()
                md5_gen.update(string)
                md5 = md5_gen.digest()
                Md5Cache._str2md5[string] = md5
                Md5Cache._md52str[md5] = string
        return md5

    @staticmethod
    def get_str(md5):
        return Md5Cache._md52str.get(md5)

_NYXCORE_MAX_INDEX = 100000
class IndexCache(object):
    _str2index = {}
    _index2str = {}

    @staticmethod
    def get_str(index):
        return IndexCache._index2str.get(index)

    @staticmethod
    def get_index(string):
        return IndexCache._str2index.get(string, -1)

class CodecEncoder(object):
    """md5或index的Encoder"""
    def __init__(self):
        self._str2index = {}

    def reset(self):
        self._str2index = {}

    def add_index(self, md5, index):
        if _NYXCORE_DEBUG:
            string = md5
        else:
            string = Md5Cache.get_str(md5)
        _logger.debug('CodecEncoder.add_index: %s, %s', string, index)
        if string and index > 0:
            self._str2index[string] = index

    def encode(self, md5_index, string):
        index = IndexCache.get_index(string)
        if index <= 0:
            index = self._str2index.get(string, 0)

        if index > 0:
            md5_index.index = index
        else:
            if _NYXCORE_DEBUG:
                md5_index.md5 = string
            else:
                md5_index.md5 = Md5Cache.get_md5(string)
        return md5_index

    def raw_encode(self, string):
        index = IndexCache.get_index(string)
        if index <= 0:
            index = self._str2index.get(string, 0)

        if index > 0:
            return index, ''
        else:
            if _NYXCORE_DEBUG:
                return 0, string
            else:
                return 0, Md5Cache.get_md5(string)
