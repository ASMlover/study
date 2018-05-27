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

import zlib
import bisect
from hashlib import md5, sha1

class HashRing(object):
    __hash_methods = {
        "crc32": lambda x: zlib.crc32(x) & 0xffffffff,
        "md5": lambda x: long(md5(x).hexdigest(), 16),
        "sha1": lambda x: long(sha1(x).hexdigest(), 16),
    }
    def __init__(self, nodes=[], replicas=128, hash_fn='crc32'):
        self._hash_fn = HashRing.__hash_methods[hash_fn]
        self._nodes = []
        self._replicas = replicas
        self._ring = {}
        self._sorted_keys = []

        for n in nodes:
            self.insert_node(n)

    def insert_node(self, node):
        self._nodes.append(node)
        for x in xrange(self._replicas):
            ring_key = self._hash_fn('%s@%s' % (node, x))
            self._ring[ring_key] = node
            self._sorted_keys.append(ring_key)
        self._sorted_keys.sort()

    def remove_node(self, node):
        self._nodes.remove(node)
        for x in xrange(self._replicas):
            ring_key = self._hash_fn('%s@%s' % (node, x))
            self._ring.pop(ring_key, None)
            self._sorted_keys.remove(ring_key)

    def _get_node_pos(self, key):
        if len(self._ring) == 0:
            return None, None

        crc = self._hash_fn(key)
        index = bisect.bisect(self._sorted_keys, crc)
        index = min(index, self._replicas * len(self._nodes) - 1)
        return index, self._ring[self._sorted_keys[index]]

    def get_node(self, key):
        _, n = self._get_node_pos(key)
        return n

    def iter_nodes(self, key):
        if len(self._ring) == 0:
            yield None, None

        pos, _ = self._get_node_pos(key)
        for k in self._sorted_keys[pos:]:
            yield k, self._ring[k]

def main():
    hr = HashRing(['node1', 'node2', 'node3', 'node4'])

    def _show_getnode(n):
        print '################## GET_NODE ###################'
        for x in xrange(n):
            print x, '==>', hr.get_node(str(x))

    _show_getnode(10)
    hr.insert_node('node5')
    _show_getnode(10)

    for k, n in hr.iter_nodes('1'):
        print k, n

if __name__ == '__main__':
    main()
