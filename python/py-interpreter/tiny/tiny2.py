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

from __future__ import print_function

class Interpreter(object):
    def __init__(self):
        self.stacks = []
        self.env = {}

    def _ir_load_val(self, number):
        self.stacks.append(number)

    def _ir_print_val(self):
        value = self.stacks.pop()
        print(value)

    def _ir_add_val(self):
        x = self.stacks.pop()
        y = self.stacks.pop()
        self.stacks.append(x + y)

    def _ir_store_var(self, name):
        value = self.stacks.pop()
        self.env[name] = value

    def _ir_load_var(self, name):
        value = self.env[name]
        self.stacks.append(value)

    def parse_argument(self, inst, argument, what_to_exec):
        numbers = ['ir_load_val']
        names = ['ir_load_var', 'ir_store_var']

        if inst in numbers:
            argument = what_to_exec['numbers'][argument]
        elif inst in names:
            argument = what_to_exec['names'][argument]
        return argument

    def execute(self, what_to_exec):
        insts = what_to_exec['insts']
        for inst, arg in insts:
            arg = self.parse_argument(inst, arg, what_to_exec)
            byte_method = getattr(self, '_%s' % inst)
            if arg is None:
                byte_method()
            else:
                byte_method(arg)

def main():
    what_to_exec = {
        'insts': [
            ('ir_load_val', 0),
            ('ir_store_var', 0),
            ('ir_load_val', 1),
            ('ir_store_var', 1),
            ('ir_load_var', 0),
            ('ir_load_var', 1),
            ('ir_add_val', None),
            ('ir_print_val', None),
        ],
        'numbers': [1, 34],
        'names': ['x', 'y'],
    }

    inter = Interpreter()
    inter.execute(what_to_exec)

if __name__ == '__main__':
    main()
