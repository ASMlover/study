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

import atexit
import collections
import os
import random
import signal
import time
from types import FrameType


class Sampler(object):
	def __init__(self, interval: float = 0.005) -> None:
		self.interval = interval
		self.started_ts = None
		self.stack_counts = collections.defaultdict(int)

	def reset(self) -> None:
		self.started_ts = time.time()
		self.stack_counts = collections.defaultdict(int)

	def start(self) -> None:
		self.started_ts = time.time()
		try:
			signal.signal(signal.SIGVTALRM, self._sample)
		except ValueError:
			raise ValueError("Can only sample on the main thread")

		signal.setitimer(signal.ITIMER_VIRTUAL, self.interval)
		atexit.register(self.stop)

	def stop(self) -> None:
		self.reset()
		signal.setitimer(signal.ITIMER_VIRTUAL, 0)

	def print_stats(self) -> None:
		if self.started_ts is None:
			return

		elapsed = time.time() - self.started_ts
		print(f"elapsed {elapsed}")
		print(f"granularity {self.interval}")
		ordered_stackss = sorted(self.stack_counts.items(), key=lambda x: x[1], reverse=True)
		for frame, count in ordered_stackss:
			print(f"{frame} {count}")

	def _sample(self, signum: int, frame: FrameType) -> None:
		stack = []
		while frame is not None:
			stack.append(self._format_frame(frame))
			frame = frame.f_back

		stack = ';'.join(reversed(stack))
		self.stack_counts[stack] += 1
		signal.setitimer(signal.ITIMER_VIRTUAL, self.interval)

	def _format_frame(self, frame: FrameType) -> str:
		func_code = frame.f_code
		return f"{func_code.co_name} ({func_code.co_filename}:{func_code.co_firstlineno})"
