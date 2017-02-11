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

import asyncore
import errno
import heapq
import sys
import time
import traceback

_nyxcore_tasks = []
_nyxcore_ncancelled = 0

def _nyxcore_remove_cancelled_tasks():
    """移除取消的任务，重新生成heap"""
    global _nyxcore_tasks, _nyxcore_ncancelled

    temp_tasks = []
    for task in _nyxcore_tasks:
        if not task._cancelled:
            temp_tasks.append(task)
    _nyxcore_tasks = temp_tasks
    heapq.heapify(_nyxcore_tasks)
    _nyxcore_ncancelled = 0

class DelayCaller(object):
    """延迟调用的caller"""
    def __init__(self, seconds, target, *args, **kwargs):
        assert callable(target), '%s is un-callable' % target
        assert sys.maxint >= seconds >= 0, '%s is not >= 0' % seconds

        self._delay = seconds
        self._target = target
        self._args = args
        self._kwargs = kwargs
        self._err_fn = kwargs.pop('err_fn', None)
        self._repush = False
        self._timeout = time.time() + self._delay
        self._cancelled = False
        self._expired = False
        heapq.heappush(_nyxcore_tasks, self)

    def __lt__(self, other):
        return self._timeout < other._timeout

    def __le__(self, other):
        return self._timeout <= other._timeout

    def call(self):
        """调用这个caller"""
        assert not self._cancelled, 'this caller already cancelled'

        try:
            try:
                self._target(*self._args, **self._kwargs)
            except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
                raise
            except:
                if self._err_fn is not None:
                    self._err_fn()
                else:
                    raise
        finally:
            if not self._cancelled:
                self.expire()

    def reset(self):
        assert not self._cancelled, 'this caller already cancelled'

        self._timeout = time.time() + self._delay
        self._repush = True

    def delay(self, seconds):
        assert not self._cancelled, 'this caller already cancelled'
        assert sys.maxint >= seconds >= 0, '%s int not >= 0' % seconds

        self._delay = seconds
        new_timeout = time.time() + self._delay
        if new_timeout > self._timeout:
            self._timeout = new_timeout
            self._repush = True
        else:
            # FIXME: should improved
            self._timeout = new_timeout
            heapq.heapify(_nyxcore_tasks)

    def cancel(self):
        assert not self._cancelled, 'this caller already cancelled'
        assert not self._expired, 'this caller already expired'

        self._cancelled = True
        del self._target, self._args, self._kwargs, self._err_fn

        global _nyxcore_tasks, _nyxcore_ncancelled
        _nyxcore_ncancelled += 1
        if _nyxcore_ncancelled > 10 and float(_nyxcore_ncancelled) / len(_nyxcore_tasks) > 0.25:
            _nyxcore_remove_cancelled_tasks()

    def expire(self):
        assert not self._cancelled, 'this caller already cancelled'
        assert not self._expired, 'this caller already expired'

        self._expired = True
        del self._target, self._args, self._kwargs, self._err_fn

class CycleCaller(DelayCaller):
    """循环调用caller"""
    def call(self):
        assert not self._cancelled, 'this caller already cancelled'

        raised = False
        try:
            try:
                self._target(*self._args, **self._kwargs)
            except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
                raised = True
                raise
            except:
                if self._err_fn is not None:
                    self._err_fn()
                else:
                    raised = True
                    raise
        finally:
            if not self._cancelled:
                if raised:
                    self.cancel()
                else:
                    self._timeout = time.time() + self._delay
                    heapq.heappush(_nyxcore_tasks, self)

if __name__ == '__main__':
    caller = DelayCaller(1, lambda x: x)
