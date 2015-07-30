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

import os

from eLisp.lexer import Lexer
from eLisp.model import Pair, Symbol, EmptyList
from eLisp.parser import Parser

def cons(head, tail):
    return Pair(head, tail)

def car(pair):
    return pair.head

def cdr(pair):
    return pair.tail

def tolist(*expr):
    def inner(args):
        if not args:
            return EmptyList
        return cons(args[0], inner(args[1:]))
    return inner(expr)

caar = lambda pair: car(car(pair))
cadr = lambda pair: car(cdr(pair))
cddr = lambda pair: cdr(cdr(pair))
caddr = lambda pair: car(cdr(cdr(pair)))
cadar = lambda pair: car(cdr(car(pair)))
cdddr = lambda pair: cdr(cdr(cdr(pair)))
caadr = lambda pair: car(car(cdr(pair)))
cdadr = lambda pair: cdr(car(cdr(pair)))
cadddr = lambda pair: car(cdr(cdr(cdr(pair))))

def is_tagged_list(expr, tag):
    if isinstance(expr, Pair) and car(expr) == tag:
        return True
    return False

def is_symbol(expr):
    return isinstance(expr, Symbol)

def pair_to_list(pair):
    result = []
    if pair is EmptyList:
        return result

    head, tail = pair.head, pair.tail
    result.append(head)
    while tail is not EmptyList:
        head, tail = tail.head, tail.tail
        result.append(head)

    return result

def is_load(expr):
    return is_tagged_list(expr, Symbol('load'))

def load(interpreter, expression):
    '''
    read expressions and definitions from file
    '''
    filepath = cadr(expression).val
    data = open(os.path.abspath(filepath)).read()
    parser = Parser(Lexer(data))

    for expr in parser.Parse():
        interpreter.interpret(expr)
