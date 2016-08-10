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

class Interpreter(object):
    def __init__(self):
        self.stack = []

    def _byte_load(self, value):
        self.stack.append(value)

    def _byte_print(self):
        value = self.stack.pop()
        print(value)

    def _byte_add(self):
        x = self.stack.pop()
        y = self.stack.pop()
        self.stack.append(x + y)

    def run_code(self, codes):
        instructions = codes['instructions']
        datas = codes['datas']
        for step in instructions:
            instruction, argument = step
            if instruction == 'load':
                self._byte_load(datas[argument])
            elif instruction == 'add':
                self._byte_add()
            elif instruction == 'print':
                self._byte_print()

def main():
    codes = {
        'instructions': [
            ('load', 0),
            ('load', 1),
            ('add', None),
            ('print', None)
        ],
        'datas': [7, 56]
    }
    vm = Interpreter()
    vm.run_code(codes)

if __name__ == '__main__':
    main()
