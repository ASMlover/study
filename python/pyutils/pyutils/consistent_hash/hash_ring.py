#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2018 ASMlover. All rights reserved.
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

import math
import sys
from bisect import bisect

if sys.version_info >= (2, 5):
    import hashlib
    md5_constructor = hashlib.md5
else:
    import md5
    md5_constructor = md5.new

class HashRing(object):
    def __init__(self, nodes=None, weights=None):
        self.ring = {}
        self._sorted_keys = []

        self.nodes = nodes
        if not weights:
            weights = {}
        self.weights = weights

        self._generate_circle()

    def _generate_circle(self):
        total_weight = 0
        for node in self.nodes:
            total_weight += self.weights.get(node, 1)

        for node in self.nodes:
            weight = 1
            if node in self.weights:
                weight = self.weights.get(node)

            factor = int(math.floor((40 * len(self.nodes) * weight) / total_weight))

            for j in xrange(factor):
                b_key = self._hash_digest('%s@%s' % (node, j))
                for i in xrange(3):
                    key = self._hash_value(b_key, lambda x: x + i * 4)
                    self.ring[key] = node
                    self._sorted_keys.append(key)
        self._sorted_keys.sort()

    def _hash_digest(self, key):
        m = md5_constructor()
        m.update(key)
        return map(ord, m.digest())

    def _hash_value(self, b_key, entry_fn):
        return ((b_key[entry_fn(3)] << 24)
                | (b_key[entry_fn(2)] << 16)
                | (b_key[entry_fn(1)] << 8)
                | b_key[entry_fn(0)])

    def gen_key(self, key):
        b_key = self._hash_digest(key)
        return self._hash_value(b_key, lambda x: x)

    def get_node_pos(self, string_key):
        if not self.ring:
            return None

        key = self.gen_key(string_key)
        nodes = self._sorted_keys
        pos = bisect(nodes, key)

        if pos == len(nodes):
            return 0
        else:
            return pos

    def get_node(self, string_key):
        pos = self.get_node_pos(string_key)
        if pos is None:
            return None
        return self.ring[self._sorted_keys[pos]]

    def iternodes(self, string_key, distinct=True):
        if not self.ring:
            yield None, None

        returned_values = set()
        def __distinct_filter(value):
            if str(value) not in returned_values:
                returned_values.add(str(value))
                return value

        pos = self.get_node_pos(string_key)
        for key in self._sorted_keys[pos:]:
            val = __distinct_filter(self.ring[key])
            if val:
                yield val

        for i, key in enumerate(self._sorted_keys):
            if i < pos:
                val = __distinct_filter(self.ring[key])
                if val:
                    yield val

def main():
    nodes = [
        'ShardStub@1',
        'ShardStub@2',
        'ShardStub@3',
        'ShardStub@4',
        'ShardStub@5',
    ]
    hr = HashRing(nodes)

    print hr.get_node('1')
    print hr.get_node('2')
    print hr.get_node('3')
    print hr.get_node('4')
    print hr.get_node('5')

if __name__ == '__main__':
    main()
