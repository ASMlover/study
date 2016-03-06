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

import PathHelper as PH
PH.addPaths('../')

import bson.objectid as objectid
import uuid

class IdCreatorBase(object):
    @staticmethod
    def genId():
        raise NotImplementedError

    @staticmethod
    def str2id(string):
        raise NotImplementedError

    @staticmethod
    def bytes2id(bytes):
        raise NotImplementedError

    @staticmethod
    def id2str(uid):
        raise NotImplementedError

    @staticmethod
    def id2bytes(uid):
        raise NotImplementedError

    @staticmethod
    def getIdType():
        raise NotImplementedError

    @staticmethod
    def isIdType(obj):
        raise NotImplementedError

class IdCreatorUuid(UuidInterface):
    @staticmethod
    def genId():
        return uuid.uuid1()

    @staticmethod
    def str2id(string):
        return uuid.UUID(string)

    @staticmethod
    def bytes2id(bytes):
        return uuid.UUID(bytes=bytes)

    @staticmethod
    def id2str(uid):
        return str(uid)

    @staticmethod
    def id2bytes(uid):
        return uid.bytes

    @staticmethod
    def getIdType():
        return uuid.UUID

    @staticmethod
    def isIdType(obj):
        return isinstance(obj, uuid.UUID)

class IdCreatorObjectId(UuidInterface):
    @staticmethod
    def genId():
        return objectid.ObjectId()

    @staticmethod
    def str2id(string):
        return objectid.ObjectId(string)

    @staticmethod
    def bytes2id(bytes):
        return objectid.ObjectId(bytes)

    @staticmethod
    def id2str(uid):
        return str(uid)

    @staticmethod
    def id2bytes(uid):
        return uid.binary

    @staticmethod
    def getIdType():
        return objectid.ObjectId

    @staticmethod
    def isIdType(obj):
        return isinstance(obj, objectid.ObjectId)

IdCreator = IdCreatorObjectId
