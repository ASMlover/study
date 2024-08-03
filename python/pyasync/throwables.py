#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2024 ASMlover. All rights reserved.
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

from typing import Any

class InvalidYield(Exception):
    __slots__ = ()

class Return(Exception):
    __slots__ = ('value')

    def __init__(self, *values: Any) -> None:
        super(Return, self).__init__()
        if not values:
            self.value = None
        elif len(values) == 1:
            self.value = values[0]
        else:
            self.value = values

class Timeout(Exception):
    __slots__ = ()

    def __init__(self, msg: str = "Timeout") -> None:
        super(Timeout, self).__init__()

class PromiseReject(Exception):
    __slots__ = ()

    def __init__(self, value: Any) -> None:
        super(PromiseReject, self).__init__(value)

    @property
    def value(self):
        return self.args

class InvalidState(Exception):
    __slots__ = ()

class CancelledError(Exception):
    __slots__ = ("cancel_outer",)

    def __init__(self) -> None:
        super(CancelledError, self).__init__()
        self.cancel_outer = True
