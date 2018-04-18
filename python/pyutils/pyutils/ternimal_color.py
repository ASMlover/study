#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2018 ASMlover. All rights reserved.
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
import platform
import sys

__WINDOWS_PLATFORM = platform.system() == 'Windows'

if __WINDOWS_PLATFORM:
    __STD_INPUTHANDLE = -10
    __STD_OUTPUTHANDLE = -11
    __STD_ERRORHANDLE = -12
    __FOREGROUND_INTENSITY = 0x08
    __FOREGROUND_BLUE = 0x09
    __FOREGROUND_GREEN = 0x0a
    __FOREGROUND_RED = 0x0c
    __FOREGROUND_YELLOW = 0x0e

    __stdout_handle = ctypes.windll.kernel32.GetStdHandle(__STD_OUTPUTHANDLE)
    def __set_terminal_color(c, h=__stdout_handle):
        return ctypes.windll.kernel32.SetConsoleTextAttribute(h, c)

    def __reset_ternimal_color():
        __set_terminal_color(__FOREGROUND_RED | __FOREGROUND_GREEN | __FOREGROUND_BLUE)

    def color_print(msg, c):
        __set_terminal_color(c)
        sys.stdout.write(msg + '\n')
        __reset_ternimal_color()
else:
    __FOREGROUND_BLUE = '\033[34m'
    __FOREGROUND_GREEN = '\033[32m'
    __FOREGROUND_RED = '\033[31m'
    __FOREGROUND_YELLOW = '\033[33m'
    __FOREGROUND_RESET = '\033[0m'

    def color_print(msg, c):
        sys.stdout.write(c)
        sys.stdout.write(msg + '\n')
        sys.stdout.write(__FOREGROUND_RESET)

if __name__ == '__main__':
    color_print("Hello, world!", __FOREGROUND_RED)
    color_print("Hello, world!", __FOREGROUND_GREEN)
    color_print("Hello, world!", __FOREGROUND_BLUE)
    color_print("Hello, world!", __FOREGROUND_YELLOW)
