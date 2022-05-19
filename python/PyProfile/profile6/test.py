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

import cpprofile as pprof

class TestEntry(object):
    def __init__(self) -> None:
        self.nothing = NothingEvent()
        self.scanning = ScanningEvent('../')
        self.sorting = SortingEvent(max_number=random.randint(500000, 1000000))

    def entry(self) -> None:
        self.nothing.do_event()
        self.scanning.do_event()
        self.sorting.do_event()

class TestEvent(object):
    def do_event(self) -> None:
        pass

class NothingEvent(TestEvent):
    def do_event(self) -> None:
        super(NothingEvent, self).do_event()

class ScanningEvent(TestEvent):
    def __init__(self, scan_filepath: str = './') -> None:
        super(ScanningEvent, self).__init__()
        self.scan_filepath = scan_filepath

    def do_scanning(self) -> List[str]:
        scanning_files = []
        for dirpath, dirs, files in os.walk(self.scan_filepath):
            for filename in files:
                scanning_files.append(os.path.join(dirpath, filename))
        return scanning_files

    def do_event(self) -> None:
        self.do_scanning()

class SortingEvent(TestEvent):
    def __init__(self, array_len: int = 5000, max_number: int = 100000) -> None:
        super(SortingEvent, self).__init__()
        self.array_len = array_len
        self.max_number = max_number
        self.random_nums  = []

    def do_generating(self) -> None:
        self.random_nums = [random.randint(1, self.max_number) for _ in range(self.array_len)]

    def do_sorting(self) -> None:
        self.random_nums.sort()

    def do_event(self) -> None:
        self.do_generating()
        self.do_sorting()

def test() -> None:
    pprof.startup(8099)
    pprof.enable()

    test_entry = TestEntry()
    while True:
        test_entry.entry()

        time.sleep(0.033)
        pprof.frame()

if __name__ == '__main__':
    test()
