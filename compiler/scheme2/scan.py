#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2015 ASMlover. All rights reserved.
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

import codecs

class Scan(object):
    def __init__(self, input_file):
        self.file = input_file
        self.line = ''
        self.pos = 0
        self.line_pos = 0
        self.char_pos = 0
    
    def next(self):
        if len(self.line) - self.pos == 0:
            self._next_line()
            if len(self.line) - self.pos == 0:
                return False
        self.pos += 1
        self.char_pos += 1
        return True

    def backup(self):
        if self.pos > 0:
            self.pos -= 1
            self.char_pos -= 1
        else:
            raise Exception('cannot backup')

    def get(self):
        return self.line[self.pos - 1]

    def emit(self):
        s = self.line[:self.pos]
        self.line = self.line[self.pos:]
        self.pos = 0
        return s

    def len(self):
        return self.pos

    def get_pos(self):
        return (self.line_pos, self.char_pos)

    def _next_line(self):
        self.line += self.file.readline()
        self.line_pos += 1
        self.pos = 0
