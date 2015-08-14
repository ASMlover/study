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

import re
import string

from atom import Symbol, String
from number import Number, Integral, LongInt, Float
from lisp import Lisp
from seq import List

DELIM = string.whitespace + Lisp.SPECIAL

class Reader(object):
    def __init__(self, source=None):
        self.raw_source = source
        self.index = 0
        self.length = 0
        self.sexpr = []

        if source:
            self.sexpr = self.get_sexpr()

    def get_sexpr(self, source=None):
        if source:
            self.raw_source = source
            self.length = len(self.raw_source)
            self.index = 0

        token = self.get_token()
        expr = None

        if token == ')':
            raise ValueError('Unexpected right paren')
        elif token == '(':
            expr = []
            token = self.get_token()

            while token != ')':
                if token == '(':
                    self.prev()
                    expr.append(self.get_sexpr())
                elif token == None:
                    raise ValueError(
                        'Invalid end of expression:', self.raw_source)
                else:
                    expr.append(token)

                token = self.get_token()
            return Lisp(expr)
        else:
            return token

    def get_token(self):
        if self.index >= self.length:
            return None

        # skip whitespace
        while self.index < self.length and self.current() in string.whitespace:
            self.next()

        if self.index == self.length:
            return None

        if self.current() in Lisp.SPECIAL:
            self.next()
            return self.previous()
        elif self.current() == '"':
            # parse a string
            s = ''
            self.next()
            while self.current() != '"' and self.index < self.length:
                s = s + self.current()
                self.next()
            self.next()
            return String(s)
        else:
            token_str = ''

            while self.index < self.length - 1:
                if self.current() in DELIM:
                    break
                else:
                    token_str = token_str + self.current()
                    self.next()
            if self.current() not in DELIM:
                token_str = token_str + self.current()
                self.next()

            if Integral.REGEX.match(token_str):
                return Integral(int(token_str))
            elif Float.REGEX.match(token_str):
                return Float(float(token_str))
            elif LongInt.REGEX.match(token_str):
                return LongInt(int(token_str))
            else:
                return Symbol(token_str)

        return None

    def prev(self):
        self.index -= 1

    def next(self):
        self.index += 1

    def current(self):
        return self.raw_source[self.index]

    def previous(self):
        return self.raw_source[self.index - 1]
