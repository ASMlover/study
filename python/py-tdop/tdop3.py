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

class SymbolBase(object):
    id = None # node/token type name
    value = None # used by literals
    node1 = node2 = node3 = None # used by tree nodes

    def nud(self):
        raise SyntaxError('Syntax error: %s' % self.id)

    def led(self, left):
        raise SyntaxError('Unknown operator %s' % self.id)

    def __repr__(self):
        if self.id == '(name)' or self.id == '(literal)':
            return '(%s %s)' % (self.id[1:-1], self.value)
        out = map(str, filter(None, (self.id, self.node1, self.node2, self.node3)))
        return '(' + ' '.join(out) + ')'

symbol_table = {}

def symbol(id, bp=0):
    try:
        s = symbol_table[id]
    except KeyError:
        class s(SymbolBase):
            pass
        s.__name__ = 'symbol-%s' % str(id)
        s.id = id
        s.lbp = bp
        symbol_table[id] = s
    else:
        s.lbp = max(bp, s.lbp)
    return s

def tokenize(program):
    for number, operator in token_pattern.findall(program):
        if number:
            symbol = symbol_table['(literal)']
            s = symbol()
            s.value = number
            yield s
        else:
            symbol = symbol_table.get(operator)
            if not symbol:
                raise SyntaxError('unknown operator')
            yield symbol()
    symbol = symbol_table['(end)']
    yield symbol()

def infix(id, bp):
    def led(self, left):
        self.node1 = left
        self.node2 = expression(bp)
        return self
    symbol(id, bp).led = led

def prefix(id, bp):
    def nud(self):
        self.node1 = expression(bp)
        self.node2 = None
        return self
    symbol(id).nud = nud

def infix_r(id, bp):
    def led(self, left):
        self.node1 = left
        self.node2 = expression(bp - 1)
        return self
    symbol(id, bp).led = led

symbol('(literal)').nud = lambda self: self
prefix('+', 100); prefix('-', 100)
infix('+', 10); infix('-', 10)
infix('*', 20); infix('/', 20)
infix_r('**', 30)
symbol('(end)')

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

def parse(program):
    global curr, next_token
    next_token = tokenize(program).next
    curr = next_token()
    return expression()

if __name__ == '__main__':
    print parse('1')
    print parse('-45')
    print parse('1+2-3')
    print parse('1+2*3')
    print parse('1+2*3/4')
    print parse('2**3**4')
