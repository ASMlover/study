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

import timeit
import typing
import coroutine
from typing import Generator


class GrepHandler(object):
    def __init__(self, pattern: str, target: Generator[None, str, None]) -> None:
        self.pattern = pattern
        self.target = target

    def send(self, line: str) -> None:
        if self.pattern in line:
            self.target.send(line)

@coroutine.corouine
def grep(pattern: str, target: Generator[None, str, None]) -> Generator[None, str, None]:
    while True:
        line = (yield)
        if pattern in line:
            target.send(line)

@coroutine.corouine
def null() -> Generator[None, str, None]:
    while True:
        line = (yield)

line = "python is nice"
p1 = grep('python', null())
p2 = GrepHandler('python', null())

if __name__ == '__main__':
    print(timeit.timeit("p1.send(line)", "from __main__ import line, p1"))
    print(timeit.timeit("p2.send(line)", "from __main__ import line, p2"))
