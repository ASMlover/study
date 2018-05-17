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

from bisect import bisect
from ring_ketama import KetamaRing
from ring_meta import MetaRing

class HashRing(object):
    def __init__(self, nodes=[], **kwds):
        hash_fn = kwds.get('hash_fn')
        vnodes = kwds.get('vnodes')
        weight_fn = kwds.get('weight_fn')

        if hash_fn == 'ketama':
            if vnodes is None:
                vnodes = 40
            self._ring = KetamaRing()
        else:
            if vnodes is None:
                vnodes = 160
            self._ring = MetaRing(hash_fn)

        self._default_vnodes = vnodes
        self._hash_fn = self._ring.hash_value

        if weight_fn and not hasattr(weight_fn, '__call__'):
            raise TypeError('`weight_fn` should be a callable function')
        self._weight_fn = weight_fn

        if self._configure_nodes(nodes):
            self._ring._create_ring(self._ring._nodes.iteritems())

    def _configure_nodes(self, nodes):
        if isinstance(nodes, str):
            nodes = [nodes]
        elif not isinstance(nodes, (list, dict)):
            raise ValueError('nodes configuration should be a list or a dict')

        conf_changed = False
        for node in nodes:
            conf = {
                'hostname': node,
                'instance': None,
                'nodename': node,
                'port': None,
                'vnodes': self._default_vnodes,
                'weight': 1,
            }
            current_conf = self._ring._nodes.get(node, {})
            nodename = node
            if not current_conf:
                conf_changed = True
            if isinstance(nodes, dict):
                node_conf = nodes[node]
                if isinstance(node_conf, int):
                    conf['weight'] = node_conf
                elif isinstance(node_conf, dict):
                    for k, v in node_conf.iteritems():
                        if k in conf:
                            conf[k] = v
                            if k in ('nodename', 'vnodes', 'weight'):
                                if current_conf.get(k) != v:
                                    conf_changed = True
                else:
                    raise ValueError('node configuration should be a dict or an int')

            if self._weight_fn:
                conf['weight'] = self._weight_fn(**conf)
            if current_conf.get('weight') != conf['weight']:
                conf_changed = True
            self._ring._nodes[nodename] = conf
        return conf_changed

    def __delitem__(self, nodename):
        self._ring._remove_node(nodename)
    remove_node = __delitem__

    def __getitem__(self, key):
        return self._get(key, 'instance')
    get_node_instance = __getitem__

    def __setitem__(self, nodename, conf={'weight': 1}):
        if self._configure_nodes({nodename: conf}):
            self._ring._create_ring([(nodename, self._nodes[nodename])])
    add_node = __setitem__

    def _get_pos(self, key):
        pos = bisect(self._ring._keys, self._hash_fn(key))
        if pos == len(self._ring._keys):
            return 0
        else:
            return pos

    def _get(self, key, what):
        if not self._ring._ring:
            return None

        pos = self._get_pos(key)
        if what == 'pos':
            return pos

        nodename = self._ring._ring[self._ring._keys[pos]]
        if what in ('hostname', 'instance', 'port', 'weight'):
            return self._ring._nodes[nodename][what]
        elif what == 'dict':
            return self._ring._nodes[nodename]
        elif what == 'nodename':
            return nodename
        elif what == 'tuple':
            return (self._ring._keys[pos], nodename)

    def get(self, key):
        return self._get(key, 'dict')

    def get_instance(self):
        return [c.get('instance') for c in self._ring._nodes.itervalues() if c.get('instance')]

    def get_key(self, key):
        return self._hash_fn(key)

    def get_node(self, key):
        return self._get(key, 'nodename')

    def get_node_hostname(self, key):
        return self._get(key, 'hostname')

    def get_node_port(self, key):
        return self._get(key, 'port')

    def get_node_pos(self, key):
        return self._get(key, 'pos')

    def get_node_weight(self, key):
        return self._get(key, 'weight')

    def get_nodes(self):
        return self._ring._nodes.iterkeys()

    def get_points(self):
        return [(k, self._ring._ring[k]) for k in self._ring._keys]

    def get_server(self, key):
        return self._get(key, 'tuple')

    def iternodes(self, key, distinct=True):
        if not self._ring._ring:
            yield None
        else:
            for node in self.range(key, unique=distinct):
                yield node['nodename']

    def regenerate(self):
        return self._ring._create_ring(self._ring._nodes.iteritems())

    def range(self, key, size=None, unique=True):
        all_nodes = set()
        if unique:
            size = size or len(self._ring._nodes)
        else:
            all_nodes = []

        pos = self._get_pos(key)
        for key in self._ring._keys[pos:]:
            nodename = self._ring._ring[key]
            if unique:
                if nodename in all_nodes:
                    continue
                all_nodes.add(nodename)
            else:
                all_nodes.append(nodename)
            yield self._ring._nodes[nodename]
            if len(all_nodes) == size:
                break
        else:
            for i, key in enumerate(self._ring._keys):
                if i < pos:
                    nodename = self._ring._ring[key]
                    if unique:
                        if nodename in all_nodes:
                            continue
                        all_nodes.add(nodename)
                    else:
                        all_nodes.append(nodename)
                    yield self._ring._nodes[nodename]
                    if len(all_nodes) == size:
                        break
    @property
    def conf(self):
        return self._ring._nodes
    nodes = conf

    @property
    def distribution(self):
        return self._ring._distribution

    @property
    def ring(self):
        return self._ring._ring

    @property
    def size(self):
        return len(self._ring._ring)

    @property
    def _nodes(self):
        return self._ring._nodes

    @property
    def _keys(self):
        return self._ring._keys
