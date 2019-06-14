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

token_pattern = re.compile("\s*(?:(\d+)|(\*\*|.))")

class LiteralToken(object):
    def __init__(self, value):
        self.value = value

    def nud(self):
        return self

    def __repr__(self):
        return self.value

class EndToken(object):
    lbp = 0

class OperatorAddToken(object):
    lbp = 10

    def nud(self):
        self.lhs = expression(10)
        self.rhs = None
        return self

    def led(self, left):
        self.lhs = left
        self.rhs = expression(10)
        return self

    def __repr__(self):
        if self.lhs and not self.rhs:
            return '%s' % self.lhs
        return "(add %s %s)" % (self.lhs, self.rhs)

class OperatorSubToken(object):
    lbp = 10

    def nud(self):
        self.lhs = expression(10)
        self.rhs = None
        return self

    def led(self, left):
        self.lhs = left
        self.rhs = expression(10)
        return self

    def __repr__(self):
        if self.lhs and not self.rhs:
            return '-%s' % self.lhs
        return "(sub %s %s)" % (self.lhs, self.rhs)

class OperatorMulToken(object):
    lbp = 20

    def led(self, left):
        self.lhs = left
        self.rhs = expression(20)
        return self

    def __repr__(self):
        return "(mul %s %s)" % (self.lhs, self.rhs)

class OperatorDivToken(object):
    lbp = 20

    def led(self, left):
        self.lhs = left
        self.rhs = expression(20)
        return self

    def __repr__(self):
        return "(div %s %s)" % (self.lhs, self.rhs)

class OperatorModToken(object):
    lbp = 20

    def led(self, left):
        self.lhs = left
        self.rhs = expression(20)
        return self

    def __repr__(self):
        return "(mod %s %s)" % (self.lhs, self.rhs)

class OperatorPowToken(object):
    lbp = 30

    def led(self, left):
        self.lhs = left
        self.rhs = expression(30 - 1)
        return self

    def __repr__(self):
        return "(pow %s %s)" % (self.lhs, self.rhs)

def expression(rbp=0):
    global curr
    prev = curr
    curr = next_token()

    left = prev.nud()
    while rbp < curr.lbp:
        prev = curr
        curr = next_token()
        left = prev.led(left)
    return left

def tokenize(program):
    for number, operator in token_pattern.findall(program):
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
        elif operator == '%':
            yield OperatorModToken()
        elif operator == '**':
            yield OperatorPowToken()
        else:
            raise SyntaxError("Unknown operator `%s`" % operator)
    yield EndToken()

def parse(program):
    global curr, next_token
    next_token = tokenize(program).next
    curr = next_token()
    return expression()

if __name__ == '__main__':
    print '+1 ==>', parse('+1')
    print '-1 ==>', parse('-1')
    print '1+2 ==>', parse('1+2')
    print '1+2-3 ==>', parse('1+2-3')
    print '1+2-3*4 ==>', parse('1+2-3*4')
    print '1+2-3*4/5 ==>', parse('1+2-3*4/5')
    print '1+2-3*4/5%6 ==>', parse('1+2-3*4/5%6')
    print '2**3**4 ==>', parse('2**3**4')
