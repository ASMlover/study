#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2020 ASMlover. All rights reserved.
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

from collections import Counter
from hashlib import md5
from typing import Callable, List, Dict, Optional

class MetaRing(object):
    def __init__(self, hash_fn: Optional[Callable[[str], int]] = None) -> None:
        self._distribution = Counter()
        self._keys : List[int] = []
        self._nodes : Dict[str, dict] = {}
        self._ring : Dict[int, str] = {}

        if hash_fn and not hasattr(hash_fn, "__call__"):
            raise TypeError(f"{hash_fn} should be a callable function")
        self._hash_fn : Callable[[str], int] = hash_fn or \
                (lambda key: int(md5(str(key).encode("utf-8")).hexdigest(), 16))

    def hashi(self, key: str) -> int:
        return self._hash_fn(key)

    def _create_ring(self, nodes : list) -> None:
        for node_name, node_conf in nodes:
            for w in range(0, node_conf["vnodes"] * node_conf["weight"]):
                self._distribution[node_name] += 1
                self._ring[self.hashi("%s-%s" % (node_name, w))] = node_name
            self._nodes[node_name] = node_conf
        self._keys = sorted(self._ring.keys())

    def _remove_node(self, node_name: str) -> None:
        try:
            node_conf = self._nodes.pop(node_name)
        except Exception:
            raise KeyError(f"node {node_name} not found, avaiable nodes: {self._nodes.keys()}")
        else:
            self._distribution.pop(node_name)
            for w in range(0, node_conf["vnodes"] * node_conf["weight"]):
                del self._ring[self.hashi("%s-%s" % (node_name, w))]
            self._keys = sorted(self._ring.keys())

if __name__ == "__main__":
    r = MetaRing()
    print (f"hash value is: {r.hashi('sssss')}")
