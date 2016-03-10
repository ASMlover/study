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

import binascii
import hashlib

from MarsLog.LogManager import LogManager

_logger = LogManager.getLogger('Utils.MessageCodec')
MARS_DEBUG = 0 # 为1是调试模式，rpc是明文
MARS_MAX_INDEX = 100000

class Md5Cache(object):
    str2Md5 = {}
    md52Str = {}

    @staticmethod
    def getMd5(string):
        md5 = Md5Cache.str2Md5(string, None)
        if not md5:
            if MARS_DEBUG:
                Md5Cache.str2Md5[string] = string
                Md5Cache.md52Str[string] = string
            else:
                md5Gen = hashlib.md5()
                md5Gen.update(string)
                md5 = md5Gen.digest()
                Md5Cache.str2Md5[string] = md5
                Md5Cache.md52Str[md5] = string
        return md5

    @staticmethod
    def getStr(md5):
        return Md5Cache.md52Str.get(md5)

class StaticIndexCache(object):
    str2Index = {}
    index2Str = {}

    @staticmethod
    def loadDict(dictFile):
        raise StandardError('Not implemented!')

    @staticmethod
    def getString(index):
        return StaticIndexCache.index2Str.get(index)

    @staticmethod
    def getIndex(string):
        return StaticIndexCache.str2Index.get(string, -1)

class Md5IndexDecoder(object):
    maxId = MARS_MAX_INDEX + 1
    str2Index = {}
    index2Str = {}

    @staticmethod
    def registerStr(string):
        if string in Md5IndexDecoder.str2Index:
            return
        index = Md5IndexDecoder.maxId
        Md5Cache.getMd5(string)
        Md5IndexDecoder.maxId += 1
        Md5IndexDecoder.str2Index[string] = index
        Md5IndexDecoder.index2Str[index] = string

    @staticmethod
    def decode(md5Index):
        string, r, index = Md5IndexDecoder.rawDecode(md5Index.md5, md5Index.index)
        if r:
            md5Index.index = index

        return string, r

    @staticmethod
    def rawDecode(md5, index):
        if index > 0:
            if index <= MARS_MAX_INDEX:
                string = StaticIndexCache.getString(index)
            else:
                string = Md5IndexDecoder.index2Str.get(index)
            if string is None:
                _logger.error('decode: string for index %d not found', index)
            return string, False, -1
        else:
            if MARS_DEBUG:
                string = md5
            else:
                string = Md5Cache.getStr(md5)
            if string is None:
                _logger.error('decode: MD5 %s for string not found', binascii.hexlify(md5))
                return string, False, -1
            index = StaticIndexCache.getIndex(string)
            if index <= 0:
                index = Md5IndexDecoder.str2Index.get(string, 0)
            return string, True, index

class Md5IndexEncoder(object):
    def __init__(self):
        super(Md5IndexEncoder, self).__init__()
        self.reset()

    def reset(self):
        self.str2Index = {}

    def addIndex(self, md5, index):
        pass

    def encode(self, md5Index, string):
        pass

    def rawEncode(self, string):
        pass
