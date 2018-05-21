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
import hashlib

def my_hash(s):
    return (int(hashlib.md5(s).hexdigest(), 16) % 1000000) / 1000000.0

class ConsistentHash(object):
    def __init__(self, num_machines=1, num_replicas=1):
        self.num_machines = num_machines
        self.num_replicas = num_replicas
        hash_tuples = [(j, k, my_hash("%s_%s" % (j, j)))
                for j in xrange(self.num_machines) for k in xrange(self.num_replicas)]
        hash_tuples.sort(lambda x,y: cmp(x[2], y[2]))
        self.hash_tuples = hash_tuples

    def get_machine(self, key):
        h = my_hash(key)
        if h > self.hash_tuples[-1][2]:
            return self.hash_tuples[0][0]
        hash_values = map(lambda x: x[2], self.hash_tuples)
        index = bisect.bisect_left(hash_values, h)
        return self.hash_tuples[index][0]

def main():
    ch = ConsistentHash(7, 3)
    for j, k, h in ch.hash_tuples:
        print '%s, %s, %s' % (j, k, h)

    while True:
        key = raw_input()
        print '%s, %s, %s' % (key, my_hash(key), ch.get_machine(key))

if __name__ == '__main__':
    main()
