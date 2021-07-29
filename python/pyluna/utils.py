#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2021 ASMlover. All rights reserved.
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

import functools
import traceback


def noexcept(func):
    @functools.wraps(func)
    def _noexcept_caller(*args, **kwds):
        try:
            return func(*args, **kwds)
        except Exception:
            traceback.print_exc()
    return _noexcept_caller

def noexecute(func):
    @functools.wraps(func)
    def _noexecute_caller(*args, **kwds):
        f_code = func.__code__
        print(f"{f_code.co_filename}:{f_code.co_firstlineno}: warning: `{f_code.co_name}(...)` is not execute")
    return _noexecute_caller

def deprecated(reason='has been explicitly marked deprecated'):
    def _deprecated(func):
        @functools.wraps(func)
        def _deprecated_caller(*args, **kwds):
            f_code = func.__code__
            print(f"{f_code.co_filename}:{f_code.co_firstlineno}: warning: `{f_code.co_name}(...)` is deprecated: {reason}")
            return func(*args, **kwds)
        return _deprecated_caller
    return _deprecated
