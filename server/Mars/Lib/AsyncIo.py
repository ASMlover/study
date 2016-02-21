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
from __future__ import print_function

import asyncore
import errno
import heapq
import time
import traceback
import sys

ASIO_MAX_CALLER_COUNT = 10
ASIO_MAX_CALLER_RATIO = 0.25

asioTasks = []
asioCancelledNum = 0

def _removeCancelledTasks():
    """Remove cancelled tasks and rebuild heap."""
    asioTasks = [t for t in asioTasks if not t.cancelled]
    heapq.heapify(asioTasks)
    asioCancelledNum = 0

def _scheduler():
    """Run the schedule function due to expire soon."""
    now = time.time()
    while asioTasks and now >= asioTasks[0].timeout:
        caller = heapq.heappop(asioTasks)
        if caller.cancelled:
            asioCancelledNum -= 1
            continue
        if caller.repush:
            heapq.heappush(asioTasks, caller)
            caller.repush = False
            continue

        try:
            caller.call()
        except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
            raise
        except:
            print(traceback.format_exc())

class CallerDelay(object):
    """Calls a function later."""
    def __init__(self, seconds, target, *args, **kwargs):
        """
            * seconds(int): number of seconds to deay
            * target(object): callable obejct
            * args: arguments to call it with
            * kwargs: keyword arguments to call it with; a special
              `errCaller` parameter can be passed, it's a callable
              called in case target function raise an exception.
        """
        assert callable(target), '%s is not callable.' % target
        assert sys.maxsize >= seconds >= 0, '%s is not >= 0' % seconds

        self.delayTime = seconds
        self.target = target
        self.args = args
        self.kwargs = kwargs
        self.errorCallback = kwargs.pop('errCaller', None)
        self.repush = False
        self.timeout = time.time() + self.delayTime
        self.cancelled = False
        self.expired = False
        heapq.heappush(asioTasks, self)

    def __lt__(self, other):
        return self.timeout < other.timeout

    def __le__(self, other):
        return self.timeout <= other.timeout

    def call(self):
        assert not self.cancelled, 'Already cancelled.'
        try:
            try:
                self.target(*self.args, **self.kwargs)
            except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
                raise
            except:
                if self.errorCallback:
                    self.errorCallback()
                else:
                    raise
        finally:
            if not self.cancelled:
                self.expire()

    def reset(self):
        assert not self.cancelled, 'Already cancelled.'
        self.timeout = time.time() + self.delayTime
        self.repush = True

    def delay(self, seconds):
        assert not self.cancelled, 'Already cancelled.'
        assert sys.maxint >= seconds >= 0, '%s is not >= 0' % seconds

        self.delayTime = seconds
        newTime = time.time() + self.delayTime
        if newTime > self.timeout:
            self.timeout = newTime
            self.repush = True
        else:
            self.timeout = newTime
            heapq.heapify(asioTasks)

    def cancel(self):
        assert not self.cancelled, 'Already cancelled.'
        assert not self.expired, 'Already expired.'

        self.cancelled = True
        del self.target, self.args, self.kwargs, self.errorCallback

        asioCancelledNum += 1
        if asioCancelledNum > ASIO_MAX_CALLER_COUNT and float(asioCancelledNum) / len(asioTasks) > ASIO_MAX_CALLER_RATIO:
            _removeCancelledTasks()

    def expire(self):
        assert not self.cancelled, 'Already cancelled.'
        assert not self.expired, 'Already expired.'
        self.expired = True
        del self.target, self.args, self.kwargs, self.errorCallback

class CallerCycle(CallerDelay):
    """Calls a function every x seconds."""
    def call(self):
        assert not self.cancelled, 'Already cancelled.'
        raised = False

        try:
            try:
                self.target(*self.args, **self.kwargs)
            except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
                raised = True
                raise
            except:
                if self.errorCallback:
                    self.errorCallback()
                else:
                    raised = True
                    raise
        finally:
            if not self.cancelled:
                if raised:
                    self.cancel()
                else:
                    self.timeout = time.time() + self.delayTime
                    heapq.heappush(asioTasks, self)

def closeAll(map=None, ignoreAll=False):
    """Close all scheduled functions and opened sockets."""
    if map is None:
        map = asyncore.socket_map
    for x in map.values():
        try:
            x.close()
        except OSError as x:
            if x.args[0] == errno.EBADF:
                pass
            elif not ignoreAll:
                raise
        except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
            raise
        except:
            if not ignoreAll:
                asyncore.socket_map.clear()
                del asioTasks[:]
                raise
    map.clear()

    for x in asioTasks:
        try:
            if not x.cancelled and not x.expired:
                x.cancel()
        except (KeyboardInterrupt, SystemExit, asyncore.ExitNow):
            raise
        except:
            if not ignoreAll:
                del asioTasks[:]
                raise
    del asioTasks[:]

def loop(timeout=0.1, usePoll=True, map=None, count=None):
    """Use this loop as replacement of the original asyncore.loop."""
    if usePoll and hasattr(asyncore.select, 'poll'):
        poller = asyncore.poll2
    else:
        poller = asyncore.poll

    if map is None:
        map = asyncore.socket_map

    if count is None:
        while (map or asioTasks):
            poller(timeout, map)
            _scheduler()
    else:
        while (map or asioTasks) and count > 0:
            poller(timeout, map)
            _scheduler()
            count -= 1


if __name__ == '__main__':
    """AsyncIo testing."""
    def delayTask():
        print('I will call 2.5 seconds later.')
    CallerDelay(2.5, delayTask)

    def cycleTask():
        print('I will call every second.')
    CallerCycle(1, cycleTask)

    loop()
