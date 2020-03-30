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

import time
import _nyxcore

_REPEAT_COUNT = 1
_COUNT = 90000
_puredict = {}
_safedict = _nyxcore.SafeIterDict()

def test_traverse_puredict():
    beg = time.clock()
    for x in xrange(_REPEAT_COUNT):
        for k, v in _puredict.items():
            if k % 3 == 0:
                del _puredict[k]
    print 'python - traverse native python dict: ', time.clock() - beg

def test_traverse_safedict():
    beg = time.clock()
    for x in xrange(_REPEAT_COUNT):
        for k, v in _safedict.iteritems():
            if k % 3 == 0:
                del _safedict[k]
    print 'python - traverse safe python dict: ', time.clock() - beg

def main():
    for x in xrange(_COUNT):
        _puredict[x] = x * x
        _safedict[x] = x * x

    test_traverse_puredict()
    test_traverse_safedict()

if __name__ == '__main__':
    main()
