#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2020 ASMlover. All rights reserved.
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

from enum import auto, IntEnum


class Code(IntEnum):
    CONSTANT = auto()

    NIL = auto()
    FALSE = auto()
    TRUE = auto()
    POP = auto()

    DEF_GLOBAL = auto()
    GET_GLOBAL = auto()
    SET_GLOBAL = auto()
    GET_LOCAL = auto()
    SET_LOCAL = auto()
    GET_UPVALUE = auto()
    SET_UPVALUE = auto()

    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()

    CALL_0 = auto()
    CALL_1 = auto()
    CALL_2 = auto()
    CALL_3 = auto()
    CALL_4 = auto()
    CALL_5 = auto()
    CALL_6 = auto()
    CALL_7 = auto()
    CALL_8 = auto()

    CLOSURE = auto()

    CLOSE_UPVALUE = auto()

    RETURN = auto()

class Chunk(object):
    def __init__(self):
        self.codes = []
        self.lines = []
        self.constants = []

    def codes_count(self) -> int:
        return len(self.codes)

    def get_code(self, i: int) -> Code:
        return self.codes[i]

    def get_line(self, i: int) -> int:
        return self.lines[i]

    def get_constant(self, i: int):
        return self.constants[i]

    def iter_constants(self):
        for c in self.constants:
            yield c

    def write(self, c: Code, l: int) -> int:
        self.codes.append(c)
        self.lines.append(l)
        return len(self.codes) - 1

    def add_constant(self, v) -> int:
        self.constants.append(v)
        return len(self.constants) - 1

    def write_constant(self, v, l: int) -> None:
        self.write(Code.CONSTANT, l)
        self.write(self.add_constant(v), l)

if __name__ == '__main__':
    print(f"{Code.CONSTANT}")
