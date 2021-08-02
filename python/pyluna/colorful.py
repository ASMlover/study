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

import ctypes
import enum
import platform
import sys
from typing import IO, Union

PYLUNA_WINDOWS = platform.system() == 'Windows'
if PYLUNA_WINDOWS:
    PYLUNA_STD_INPUT = -10
    PYLUNA_STD_OUTPUT = -11
    PYLUNA_STD_ERROR = -12

    pyluna_stdout: int = ctypes.windll.kernel32.GetStdHandle(PYLUNA_STD_OUTPUT)
    pyluna_stderr: int = ctypes.windll.kernel32.GetStdHandle(PYLUNA_STD_ERROR)
    def pyluna_set_terminal_color(h: int, c: int) -> int:
        return ctypes.windll.kernel32.SetConsoleTextAttribute(h, c)

    class Foreground(enum.IntEnum):
        BLACK = 0x00
        BLUE = 0x01
        GREEN = 0x02
        CYAN = BLUE | GREEN
        RED = 0x04
        MAGENTA = BLUE | RED
        YELLOW = GREEN | RED
        WHITE = BLUE | GREEN | RED
        GRAY = 0x08

        RESET = WHITE

        LIGHTBLUE = 0x09
        LIGHTGREEN = 0x0a
        LIGHTCYAN = LIGHTBLUE | LIGHTGREEN
        LIGHTRED = 0x0c
        LIGHTMAGENTA = LIGHTBLUE | LIGHTRED
        LIGHTYELLOW = LIGHTGREEN | LIGHTRED
        LIGHTWHITE = LIGHTBLUE | LIGHTGREEN | LIGHTRED
else:
    pyluna_stdout, pyluna_stderr = sys.stdout, sys.stderr
    def pyluna_set_terminal_color(h: IO, c: str) -> None:
        h.write(c)

    class Foreground(object):
        RESET = "\033[00m"

        BLACK = "\033[30m"
        BLUE = "\033[34m"
        GREEN = "\033[32m"
        CYAN = "\033[36m"
        RED = "\033[31m"
        MAGENTA = "\033[35m"
        YELLOW = "\033[33m"
        WHITE = "\033[37m"
        GRAY = "\033[90m"

        LIGHTBLUE = "\033[94m"
        LIGHTGREEN = "\033[92m"
        LIGHTCYAN = "\033[96m"
        LIGHTRED = "\033[91m"
        LIGHTMAGENTA = "\033[95m"
        LIGHTYELLOW = "\033[93m"
        LIGHTWHITE = "\033[97m"

class TerminalColorful(object):
    def __init__(self, c: Union[int, str], h: Union[int, IO] = pyluna_stdout) -> None:
        super(TerminalColorful, self).__init__()
        self.color = c
        self.handle = h

    def __enter__(self) -> Union[int, None]:
        return pyluna_set_terminal_color(self.handle, self.color)

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        pyluna_set_terminal_color(self.handle, Foreground.RESET)

def xprint(color: Union[int, str], *args, **kwds) -> None:
    with TerminalColorful(color):
        print(*args, **kwds)
