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

import signal
import time
from types import FrameType
from typing import Any, Callable


def timeout(seconds: int = 1, message: str = "Execute Timeout !!!", default: Any = None) -> Callable:
	def _timeout_wrapper(func) -> Callable:
		def _timeout_handler(signum: int, frame: FrameType) -> None:
			raise TimeoutError(message)

		def _caller(*args, **kwds) -> Any:
			signal.signal(signal.SIGALRM, _timeout_handler)
			signal.alarm(seconds)
			try:
				result = func(*args, **kwds)
			except TimeoutError as ex:
				print(f"[timeout][{func.__name__}] {ex}")
				result = default
			finally:
				signal.alarm(0)
				signal.signal(signal.SIGALRM, signal.SIG_DFL)
			return result
		return _caller
	return _timeout_wrapper

def timeout_func() -> float:
	beg = time.time()
	time.sleep(5)
	return time.time() - beg

@timeout(1)
def test_with_timeout_decorator() -> float:
	return timeout_func()

def test_without_timeout_decorator() -> float:
	return timeout_func()

def main() -> None:
	r = test_without_timeout_decorator()
	print(f"[test_without_timeout_decorator] returns: {r}")

	r = test_with_timeout_decorator()
	print(f"[test_with_timeout_decorator] returns: {r}")

if __name__ == "__main__":
	main()
