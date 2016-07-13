#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

from collections import deque

class Stack(deque):
    push = deque.append

    def top(self):
        return self[-1]

class Interpreter(object):
    def __init__(self):
        self.stacks = Stack()
        self.env = {}

    def _push(self, value):
        self.stacks.push(value)

    def _pop(self):
        return self.stacks.pop()

    def _popn(self, n=1):
        if n <= 1:
            return self.stacks.pop()

        values = []
        for i in range(n):
            values.append(self._pop())
        return values

    def _ir_iprint(self):
        print(self._pop())

    def _ir_iconst(self, value):
        self._push(value)

    def _ir_istore(self, name):
        self.env[name] = self._pop()

    def _ir_iload(self, name):
        self._push(self.env[name])

    def _ir_iadd(self):
        y, x = self._popn(2)
        self._push(x + y)

    def _ir_isub(self):
        y, x = self._popn(2)
        self._push(x - y)

    def _ir_imul(self):
        y, x = self._popn(2)
        self._push(x * y)

    def _ir_idiv(self):
        y, x = self._popn(2)
        self._push(x // y)

    def _ir_itruediv(self):
        y, x = self._popn(2)
        self._push(x / y)

    def _ir_imod(self):
        y, x = self._popn(2)
        self._push(x % y)

    def parse_argument(self, inst, argument, vm_codes):
        consts = ['iconst']
        names = ['istore', 'iload']

        if inst in consts:
            argument = vm_codes['consts'][argument]
        elif inst in names:
            argument = vm_codes['names'][argument]
        return argument

    def execute(self, vm_codes):
        insts = vm_codes['insts']
        for inst, arg in insts:
            arg = self.parse_argument(inst, arg, vm_codes)
            method = getattr(self, '_ir_%s' % inst)
            if arg is None:
                method()
            else:
                method(arg)

def main():
    what_to_exec = {
        'insts': [
            ('iconst', 0),
            ('istore', 0),
            ('iconst', 1),
            ('istore', 1),
            ('iload', 0),
            ('iload', 1),
            ('itruediv', None),
            ('iprint', None),
        ],
        'consts': [34, 4],
        'names': ['x', 'y'],
    }
    Interpreter().execute(what_to_exec)

if __name__ == '__main__':
    main()
