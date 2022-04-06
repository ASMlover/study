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

import os
import random
import time
from typing import List

import py_profile as pprof

class TestMain(object):
    def __init__(self) -> None:
        self.scanner: Scanner = Scanner('../')
        self.sorter: Sorter = Sorter(500000)

    @pprof.profile
    def update(self) -> None:
        self.scanner.update()
        self.sorter.update()

class Scanner(object):
    def __init__(self, scan_filepath: str = './') -> None:
        super(Scanner, self).__init__()
        self.scan_filepath: str = scan_filepath

    @pprof.profile
    def do_scanning(self) -> List[str]:
        scanning_files = []
        for dirpath, dirs, files in os.walk(self.scan_filepath):
            for filename in files:
                scanning_files.append(os.path.join(dirpath, filename))
        return scanning_files

    @pprof.profile
    def update(self) -> None:
        self.do_scanning()

class Sorter(object):
    def __init__(self, counter: int = 100000, times: int = 100) -> None:
        self.counter: int = counter
        self.times: int = times
        self.random_nums: List[int] = []

    @pprof.profile
    def do_generating(self) -> None:
        self.random_nums.append(random.randint(1, self.counter))

    @pprof.profile
    def do_sorting(self) -> None:
        self.random_nums.sort()

    @pprof.profile
    def update(self) -> None:
        for _ in range(self.times):
            self.do_generating()
            self.do_sorting()

def main():
    test = TestMain()

    test_count = 0
    while test_count < 100:
        test.update()
        time.sleep(0.01)

        test_count += 1

if __name__ == '__main__':
    main()
