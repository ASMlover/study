#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2022 ASMlover. All rights reserved.
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
import time
from typing import Any, Callable


_profile_stats = {} # {(funcname, filename, lineno): [use_tm1, use_tm2, ...], ...}

def profile(func: Callable[..., Any]) -> Callable[..., Any]:
    @functools.wraps(func)
    def _profile_caller(*args, **kwds) -> Any:
        try:
            time_beg = time.time_ns()
            return func(*args, **kwds)
        finally:
            time_end, func_code = time.time_ns(), func.__code__
            funcname, filename, lineno = func_code.co_name, func_code.co_filename, func_code.co_firstlineno
            profile_times = _profile_stats.setdefault((funcname, filename, lineno), [])
            profile_times.append(time_end - time_beg)
    return _profile_caller

def start_stats():
    pass

def print_stats():
    pass
