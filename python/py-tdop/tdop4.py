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
    id = None
    value = None
    node1 = node2 = node3 = None

    def nud(self):
        raise SyntaxError("syntax error: %s" % self.id)

    def led(self, left):
        raise SyntaxError("Unknown operator: %s" % self.id)

    def __repr__(self):
        if self.id == "(name)" or self.id == "(literal)":
            return "(%s %s)" % (self.id[1:-1], self.value)
        out = map(str, filter(None, (self.id, self.node1, self.node2, self.node3)))
        return "(" + " ".join(out) + ")"

symbol_table = {}
def symbol(id, bp=0):
    try:
        s = symbol_table[id]
    except KeyError:
        class s(SymbolBase):
            pass
        s.__name__ = "symbol-%s" % id
        s.id = id
        s.lbp = bp
        symbol_table[id] = s
    else:
        s.lbp = max(bp, s.lbp)
    return s

def prefix(id, bp):
    def nud(self):
        self.node1 = expression(bp)
        self.node2 = None
        return self
    symbol(id).nud = nud

def infix(id, bp):
    def led(self, left):
        self.node1 = left
        self.node2 = expression(bp)
        return self
    symbol(id, bp).led = led

def infix_r(id, bp):
    def led(self, left):
        self.node1 = left
        self.node2 = expression(bp - 1)
        return self
    symbol(id, bp).led = led

def paren_expr():
    def nud(self):
        expr = expression()
        advance(')')
        return expr
    symbol('(').nud = nud
    symbol(')')

def logic_expr():
    @method(symbol('if'))
    def led(self, left):
        self.node1 = left
        self.node2 = expression()
        advance('else')
        self.node3 = expression()
        return self
    symbol('else')

def dot_expr():
    @method(symbol('.'))
    def led(self, left):
        if curr.id != '(name)':
            raise SyntaxError('expected an attribute name')
        self.node1 = left
        self.node2 = curr
        advance()
        return self

def attr_expr():
    @method(symbol('['))
    def led(self, left):
        self.node1 = left
        self.node2 = expression()
        advance(']')
        return self
    symbol(']')

def method(s):
    assert issubclass(s, SymbolBase)
    def bind(fn):
        setattr(s, fn.__name__, fn)
    return bind

def func_call():
    @method(symbol('('))
    def led(self, left):
        self.node1 = left
        self.node2 = []
        if curr.id != ')':
            while True:
                self.node2.append(expression())
                if curr.id != ',':
                    break
                advance(',')
            self.node2 = tuple(self.node2)
        advance(')')
        return self
    symbol(')'); symbol(',')

def lambda_expr():
    def arguments(args):
        while True:
            if curr.id != '(name)':
                raise SyntaxError('expected an arguments name')
            args.append(curr)
            advance()
            if curr.id != ',':
                break
            advance(',')

    @method(symbol('lambda'))
    def nud(self):
        self.node1 = []
        if curr.id != ':':
            arguments(self.node1)
        self.node1 = tuple(self.node1)
        advance(':')
        self.node2 = expression()
        return self

    symbol(':')

def constant(id):
    @method(symbol(id))
    def nud(self):
        self.id = "(literal)"
        self.value = id
        return self

symbol('lambda', 20)
symbol('if', 20)
infix_r('or', 30); infix_r('and', 40); prefix('not', 50)

infix('in', 60); infix('not', 60) # in, not in
infix('is', 60) # is, is not
infix('<', 60); infix('<=', 60)
infix('>', 60); infix('>=', 60)
infix('<>', 60); infix('!=', 60); infix('==', 60)

infix('|', 70); infix('^', 80); infix('&', 90)
infix('<<', 100); infix('>>', 100)
infix('+', 110); infix('-', 110)
infix('*', 120); infix('/', 120); infix('//', 120); infix('%', 120)

prefix('+', 130); prefix('-', 130); prefix('~', 130)
infix_r('**', 140)

symbol('.', 150); symbol('[', 150); symbol('(', 150)

symbol('(literal)').nud = lambda self: self
symbol('(name)').nud = lambda self: self
symbol('(end)')

paren_expr(); logic_expr(); dot_expr(); attr_expr(); func_call(); lambda_expr()
constant('None'); constant('True'); constant('False')

def tokenize_python(program):
    import tokenize
    from cStringIO import StringIO
    type_map = {
        tokenize.NUMBER: "(literal)",
        tokenize.STRING: "(literal)",
        tokenize.OP: "(operator)",
        tokenize.NAME: "(name)",
    }

    for t in tokenize.generate_tokens(StringIO(program).next):
        try:
            yield type_map[t[0]], t[1]
        except KeyError:
            if t[0] == tokenize.ENDMARKER:
                break
            else:
                raise SyntaxError("syntax error")
    yield "(end)", "(end)"

def tokenize(program):
    for id, value in tokenize_python(program):
        if id == "(literal)":
            symbol = symbol_table[id]
            s = symbol()
            s.value = value
        else:
            symbol = symbol_table.get(value)
            if symbol:
                s = symbol()
            elif id == "(name)":
                symbol = symbol_table[id]
                s = symbol()
                s.value = value
            else:
                raise SyntaxError("Unknown operator: %s" % id)
        yield s

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

def advance(id=None):
    global curr, next_token
    if id and curr.id != id:
        raise SyntaxError("expected %s" % id)
    curr = next_token()

def parse(program):
    global curr, next_token
    next_token = tokenize(program).next
    curr = next_token()
    return expression()

if __name__ == '__main__':
    print parse('+2')
    print parse('-3')
    print parse('1+2+3-4')
    print parse('1+2*3')
    print parse('10.56 / 56.8 + 45.33')
    print parse('"hello" + "world!"')
    print parse('23.5**45.2')
    print parse('2 << 3')
    print parse('(1+2)*3/(5-2)')
    print parse('1 if 2 else 3')
    print parse('foo.bar')
    print parse('"hello"[2]')
    print parse('hello(1, 2, 3)')
    print parse('lambda a, b: a+b')
    print parse('1 is None')
    print parse('True or False')
