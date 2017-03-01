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

import sys
import unittest

sys.path.append('../lib')
sys.path.append('../core')

from log.nyx_log import LogManager

_logger = LogManager.get_logger('NyxCore.Tests')

class NyxEngineUnittest(unittest.TestCase):
    def test_async_timer(self):
        import async_timer
        import asyncore_scheduler

        def foo():
            _logger.debug('show foo function')
        async_timer.add_timer(1, foo)
        asyncore_scheduler.loop()

    def test_entity_scanner(self):
        from common.nyx_entity import EntityScanner as ES

        class_dict = ES().scan_entity_classes('../core/common', (object, type,))
        for key, value in class_dict.iteritems():
            _logger.debug('{%s => %s}', key, value)

    def test_id_creator(self):
        from common.nyx_id import IdCreator

        for i in xrange(10):
            _logger.debug('[%d] id=%s', i, IdCreator.genid())

    def test_codec(self):
        import random
        from common.nyx_codec import Md5Cache
        from common.nyx_codec import IndexCache

        # unittest for Md5Cache
        for i in xrange(50):
            s = 'test_codec.Md5Cache#%d' % (i + 1)
            md5 = Md5Cache.get_md5(s)
            _logger.debug('{%s => %s} {%s => %s}', s, md5, md5, Md5Cache.get_str(md5))

        # unittest for IndexCache
        for i in xrange(50):
            s = 'test_codec.IndexCache#%d' % i
            IndexCache._index2str[i] = s
            IndexCache._str2index[s] = i
        for i in xrange(10):
            n = random.randint(1, 49)
            s = 'test_codec.IndexCache#%d' % n
            assert n == IndexCache.get_index(s) and s == IndexCache.get_str(n)

def main():
    suite = unittest.TestLoader().loadTestsFromTestCase(NyxEngineUnittest)
    unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == '__main__':
    main()
