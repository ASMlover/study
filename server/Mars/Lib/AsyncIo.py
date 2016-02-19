#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

import asyncore
import errno
import heapq
import time
import traceback
import sys

aioTasks = []
aioCancelledNum = 0

class CallerDelay(object):
    """call a function later"""
    def __init__(self, seconds, target, *args, **kwargs):
        assert callable(target), '%s is not callable' % target
        assert sys.maxint >= seconds >= 0, '%s is not >= 0' % seconds

        self.delayTime = seconds
        self.target = target
        self.args = args
        self.kwargs = kwargs
        self.errorBack = kwargs.pop('errorBack', None)
        self.repush = False
        self.timeout = time.time() + self.delayTime
        self.cancelled = False
        self.expired = False
        heapq.heappush(aioTasks, self)

    def __le__(self, other):
        return self.timeout <= other.timeout

    def call(self):
        assert not self.cancelled, 'Already cancelled'
        try:
            try:
                self.target(*self.args, **self.kwargs)
            except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
                raise
            except:
                if self.errorBack:
                    self.errorBack()
                else:
                    raise
        finally:
            if not self.cancelled:
                self.expire()

    def reset(self):
        assert not self.cancelled, 'Already cancelled'
        self.timeout = time.time() + self.delayTime
        self.repush = True

    def delay(self, seconds):
        assert not self.cancelled, 'Already cancelled'
        assert sys.maxint >= seconds >= 0, '%s is not >= 0' % seconds

        self.delayTime = seconds
        newTime = time.time() + self.delayTime
        if newTime > self.timeout:
            self.timeout = newTime
            self.repush = True
        else:
            self.timeout = newTime
            heapq.heapify(aioTasks)

    def cancel(self):
        pass

    def expire(self):
        pass

def loop(timeout=0.1, usePoll=True, map=None, count=None):
    pass
