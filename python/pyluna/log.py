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

import time

from colorful import Foreground as _fg
from colorful import xprint as _xprint


class Logger(object):
    def __init__(self, name: str) -> None:
        super(Logger, self).__init__()
        self.name = name

    def __repr__(self):
        return f"Logger(id:{id(self)}, name:{self.name})"

    def format_string(self, fmt: str, *args) -> str:
        return '[%s] - %s - %s' % (time.strftime('%Y-%m-%d %Y:%M:%S', time.localtime()), self.name, fmt % args)

    def debug(self, *args) -> None:
        _xprint(_fg.LIGHTGREEN, self.format_string(*args))

    def info(self, *args) -> None:
        _xprint(_fg.LIGHTWHITE, self.format_string(*args))

    def warn(self, *args) -> None:
        _xprint(_fg.LIGHTYELLOW, self.format_string(*args))

    def error(self, *args) -> None:
        _xprint(_fg.LIGHTRED, self.format_string(*args))


pyluna_loggers = {}
def get_logger(name: str = 'PyLuna') -> Logger:
    if name not in pyluna_loggers:
        pyluna_loggers[name] = Logger(name)
    return pyluna_loggers[name]
