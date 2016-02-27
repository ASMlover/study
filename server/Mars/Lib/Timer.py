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

import PathHelper as PH
PH.addPathes('../')

import AsyncIo

def addTimer(delay, func, *args, **kwargs):
    """
        * delay(float): seconds to wait.
        * func(object): callable object to call later.
        * args: arguments to call it with.
        * kwargs: keyword arguments to call it with; a special
          `errCaller` parameter can be passed: it's a callable
          called in case target function raise an exception.

        * return: a timer object can reset or cancel.
    """
    return AsyncIo.CallerDelay(delay, func, *args, **kwargs)

def addRepeatTimer(delay, func, *args, **kwargs):
    """
        * delay(float): call it every `delay` seconds.
        * func(object): callable object to call later.
        * args: arguments to call it with.
        * kwargs: keyword arguments to call it with; a special
          `errCaller` parameter can be passed: it's a callable
          called in case target function raise an exception.

        * return: a timer object can reset or cancel.
    """
    return AsyncIo.CallerCycle(delay, func, *args, **kwargs)
