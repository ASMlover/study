#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2019 ASMlover. All rights reserved.
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

class LiteralToken(object):
    def __init__(self, value):
        self.value = int(value)

    def nud(self):
        return self.value

class OperatorAddToken(object):
    lbp = 10

    def nud(self):
        return expression(100)

    def led(self, left):
        return left + expression(10)

class OperatorSubToken(object):
    lbp = 10

    def nud(self):
        return -expression(100)

    def led(self, left):
        return left - expression(10)

class OperatorMulToken(object):
    lbp = 20
    def led(self, left):
        return left * expression(20)

class OperatorDivToken(object):
    lbp = 20
    def led(self, left):
        return left / expression(20)

class OperatorPowToken(object):
    lbp = 30
    def led(self, left):
        return left ** expression(30 - 1)

class EndToken(object):
    lbp = 0

def expression(rbp=0):
    global token
    t = token
    token = next()
    left = t.nud()
    while rbp < token.lbp:
        t = token
        token = next()
        left = t.led(left)
    return left

def tokenize(program):
    for number, operator in re.findall("\s*(?:(\d+)|(\*\*|.))", program):
        if number:
            yield LiteralToken(number)
        elif operator == '+':
            yield OperatorAddToken()
        elif operator == '-':
            yield OperatorSubToken()
        elif operator == '*':
            yield OperatorMulToken()
        elif operator == '/':
            yield OperatorDivToken()
        elif operator == '**':
            yield OperatorPowToken()
        else:
            raise SyntaxError('Unknown operator')
    yield EndToken()

def parse(program):
    global token, next
    next = tokenize(program).next
    token = next()

    return expression()

if __name__ == '__main__':
    print parse('1+2+3')
    print parse('1+2-3')
    print parse('1+2-3*4')
    print parse('1+2-3*4/5')
    print parse('-1+2*3')
    print parse('+56-45/2')
    print parse('2**3**4')
