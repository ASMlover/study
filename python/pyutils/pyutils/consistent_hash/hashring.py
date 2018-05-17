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

import bisect
import md5

class ConsistentHashRing(object):
    def __init__(self, replicas=100):
        self.replicas = replicas
        self._keys = []
        self._nodes = {}

    def __hash(self, key):
        return int(md5.md5(key).hexdigest(), 16)

    def __repl_iterator(self, nodename):
        return (self.__hash('%s@%s' % (nodename, i)) for i in xrange(self.replicas))

    def __setitem__(self, nodename, node):
        for h in self.__repl_iterator(nodename):
            if h in self._nodes:
                raise ValueError('Node %r is already present' % nodename)
            self._nodes[h] = node
            bisect.insort(self._keys, h)

    def __delitem__(self, nodename):
        for h in self.__repl_iterator(nodename):
            del self._nodes[h]
            index = bisect.bisect_left(self._keys, h)
            del self._keys[index]

    def __getitem__(self, key):
        h = self.__hash(key)
        start = bisect.bisect(self._keys, h)
        if start == len(self._keys):
            start = 0
        return self._nodes[self._keys[start]]

class Node(object):
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return 'Node<%s>{name=%s}' % (id(self), self.name)

def main():
    ch = ConsistentHashRing(100)
    ch['node1'] = Node('host1')
    ch['node2'] = Node('host2')

    n = ch['node2']
    print n

if __name__ == '__main__':
    main()
