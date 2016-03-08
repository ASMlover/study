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
