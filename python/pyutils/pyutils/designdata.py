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

class DesignData(dict):
    __slots__ = ('indexes',)

    def __init__(self, indexes, data):
        super(DesignData, self).__init__(data)
        self.indexes = indexes

    def get(self, key, default=None):
        try:
            return self[key]
        except:
            return default

    def getsub(self, key, subkey, default=None):
        try:
            return self.__getitem__(self, key)[self.indexes[subkey]]
        except:
            return default

    def getsubs(self, key, *subkeys):
        raw_item = self.get_raw_item(key)
        return (raw_item[self.indexes[subkey]] for subkey in subkeys)

    def __getitem__(self, key):
        item = dict.__getitem__(self, key)
        if isinstance(item, dict):
            return dict([(subkey, item[index]) for subkey, index in self.indexes.iteritems() if index in item])
        else:
            return dict([(subkey, item[index]) for subkey, index in self.indexes.iteritems()])

    def get_raw_item(self, key):
        return dict.__getitem__(self, key)

    def iteritems(self, *subkeys):
        if not subkeys:
            for key in self:
                yield key, self[key]
        elif len(subkeys) == 1:
            subkey = subkeys[0]
            for key in self:
                yield key, self.getsub(key, subkey)
        else:
            for key in self:
                item = self[key]
                yield key, dict([(subkey, item.get(self.indexes[subkey])) for subkey in subkeys])

    def itervalues(self):
        for key in self:
            yield self[key]
