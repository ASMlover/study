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
import queue
import sched
import signal
import threading
import time
from types import FrameType
from typing import Callable


class TimerProxy(object):
	def __init__(self, is_repeat: bool, delay: float, func: Callable) -> None:
		super(TimerProxy, self).__init__()

		self.delay = delay
		self.func = func
		self.is_repeat = is_repeat

		self.timer = threading.Timer(self.delay, self.on_worker)
		self.timer.start()

	def on_worker(self) -> None:
		if self.func:
			self.func()

		if self.is_repeat:
			self.timer = threading.Timer(self.delay, self.on_worker)
			self.timer.start()

	def cancel(self) -> None:
		self.is_repeat = False
		if self.timer:
			self.timer.cancel()
			self.timer = None

class Timer(object):
	@staticmethod
	def addTimer(delay: float, func: Callable) -> TimerProxy:
		return TimerProxy(False, delay, func)

	@staticmethod
	def addRepeatTimer(delay: float, func: Callable) -> TimerProxy:
		return TimerProxy(True, delay, func)


class LatencyTracker(object):
	TRACK_DELTATIME = 0.005
	_instance = None

	def __init__(self, track_interval: float = 0.1) -> None:
		super(LatencyTracker, self).__init__()

		self.stopflag = False
		self.feed_queue = queue.Queue()
		self.feed_timestamp = 0

		self.track_interval = track_interval
		self.track_timer = None

	@classmethod
	def get_instance(cls):
		if cls._instance is None:
			cls._instance = LatencyTracker()
		return cls._instance

	def format_frame(self, frame: FrameType) -> str:
		func_code = frame.f_code
		return f"{func_code.co_name} ({func_code.co_filename}:{func_code.co_firstlineno})"

	def on_watchdog_worker(self) -> None:
		now = time.time()
		if self.feed_queue.empty():
			if now - self.feed_timestamp > self.track_interval + self.TRACK_DELTATIME:
				os.kill(os.getpid(), signal.SIGUSR1)
		else:
			feed = self.feed_queue.get(block=True)
			# print(f"######### [LatencyTracker][on_watchdog_worker] {self.feed_queue} - {feed}")
			self.feed_timestamp = now

		if not self.stopflag:
			threading.Timer(self.track_interval, self.on_watchdog_worker).start()

	def on_signal_handler(self, signum: int, frame: FrameType) -> None:
		stack = []
		while frame is not None:
			stack.append(self.format_frame(frame))
			frame = frame.f_back
		stack.append(f"[LatencyTracker] Callstack:")

		stackstr = "\n > ".join(reversed(stack))
		print(f"{stackstr}")

	def on_track_timer(self):
		if self.feed_queue.empty():
			self.feed_queue.put("feed")
			# print(f"######### [LatencyTracker][on_track_timer] {self.feed_queue} - feed")

	def start(self, track_interval: float = 0.1) -> None:
		self.stopflag = False
		self.feed_timestamp = time.time()
		self.track_interval = track_interval

		signal.signal(signal.SIGUSR1, self.on_signal_handler)
		threading.Timer(self.track_interval, self.on_watchdog_worker).start()

		if self.track_timer:
			self.track_timer.cancel()
		self.track_timer = Timer.addRepeatTimer(self.track_interval, self.on_track_timer)

	def stop(self) -> None:
		self.stopflag = True

		if self.track_timer:
			self.track_timer.cancel()
			self.track_timer = None

def start_tracker(track_interval: float = 0.1) -> None:
	LatencyTracker.get_instance().start(track_interval)

def stop_tracker():
	LatencyTracker.get_instance().stop()
