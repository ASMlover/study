#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2023 ASMlover. All rights reserved.
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
from typing import List

import py_profiler as pprof


class TestEntry(object):
	def __init__(self) -> None:
		self.dummy = DummyEvent()
		self.scanning = ScanningEvent("../")
		self.sorting = SortingEvent(max_number=random.randint(500000, 1000000))

	@pprof.profile
	def run(self) -> None:
		self.dummy.run_event()
		self.scanning.run_event()
		self.sorting.run_event()

class TestEvent(object):
	def run_event(self) -> None:
		pass

class DummyEvent(TestEvent):
	@pprof.profile
	def run_event(self) -> None:
		super(DummyEvent, self).run_event()

class ScanningEvent(TestEvent):
	def __init__(self, scaning_filepath: str = "./") -> None:
		super(ScanningEvent, self).__init__()
		self.scaning_filepath = scaning_filepath

	@pprof.profile
	def run_event(self) -> None:
		self.run_scanning()

	@pprof.profile
	def run_scanning(self) -> List[str]:
		scanning_files = []
		for dirpath, dirs, files in os.walk(self.scaning_filepath):
			for filename in files:
				scanning_files.append(os.path.join(dirpath, filename))
		return scanning_files

class SortingEvent(TestEvent):
	def __init__(self, array_count: int = 5000, max_number: int = 100000) -> None:
		super(SortingEvent, self).__init__()
		self.random_array = []
		self.array_count = array_count
		self.max_number = max_number

	@pprof.profile
	def run_event(self) -> None:
		self.run_generating()
		self.run_sorting()

	@pprof.profile
	def run_generating(self) -> None:
		self.random_array = [random.randint(1, self.max_number) for _ in range(self.array_count)]

	@pprof.profile
	def run_sorting(self) -> None:
		self.random_array.sort()
