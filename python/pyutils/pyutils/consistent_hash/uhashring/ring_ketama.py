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
from collections import Counter
from hashlib import md5

class KetamaRing(object):
    def __init__(self):
        self._distribution = Counter()
        self._keys = []
        self._nodes = {}
        self._ring = {}
        self._replicas = 4

    @staticmethod
    def _listbytes(data):
        return map(ord, data)

    def _hash_value_weight_generator(self, node_name, node_conf):
        ks = (node_conf['vnodes'] * len(self._nodes) * node_conf['weight']) // self._weight_sum
        for w in xrange(ks):
            w_node_name = '%s@%s' % (node_name, w)
            for i in xrange(self._replicas):
                yield self.hash_value(w_node_name, replica=i)

    def _create_ring(self, nodes):
        weight_sum = 0
        for node_conf in self._nodes.itervalues():
            weight_sum += node_conf['weight']
        self._weight_sum = weight_sum

        self._distribution = Counter()
        self._keys = []
        self._ring = {}
        for node_name, node_conf in self._nodes.iteritems():
            for h in self._hash_value_weight_generator(node_name, node_conf):
                self._ring[h] = node_name
                bisect.insort(self._keys, h)
                self._distribution[node_name] += 1

    def _remove_node(self, node_name):
        try:
            del self._nodes[node_name]
        except Exception:
            raise KeyError('node `%s` not found, available nodes: %s' % (node_name, self._nodes.keys()))
        else:
            self._create_ring(self._nodes)

    def hash_value(self, key, replica=0):
        dh = self._listbytes(md5(str(key).encode('utf-8')).digest())
        rd = replica * 4
        return ((dh[3 + rd] << 24) | (dh[2 + rd] << 16) | (dh[1 + rd] << 8) | dh[0 + rd])
