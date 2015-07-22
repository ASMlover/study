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

import operator as op
import sys

Symbol = str
Number = (int, float)
List = list

try:
    apply
except NameError:
    def apply(f, *args, **kwargs):
        return f(*args, **kwargs)

def get_input(*args, **kwargs):
    if sys.verson[0] == '2':
        return raw_input(*args, **kwargs)
    else:
        return input(*args, **kwargs)

class Env(dict):
    def __init__(self, values=None, outer=None):
        if values:
            self.update(values)
        self.outer = outer

    def find(self, var):
        if var in self:
            return self
        elif self.outer:
            return self.outer.find(var)
        else:
            raise NameError('unexpected variable <%s>' % var)

# built-ins
def get_builtins():
    return {
        # logical
        'not': op.not_, 

        # arithmetic
        'add': op.add,
        'sub': op.sub,
        'mul': op.mul,
        'div': op.truediv,
        'mod': op.mod,
        '+': op.add,
        '-': op.sub,
        '*': op.mul, 
        '/': op.truediv,
        '%': op.mod,

        # comparison
        'equal?': op.eq, 
        'eq?': op.is_,
        '>': op.gt,
        '<': op.lt,
        '>=': op.ge,
        '<=': op.le,

        # pairs
        'cons': lambda x, y: [x] + y, # y must be a list
        'car': lambda x: x[0],
        'cdr': lambda x: x[1:],

        # list
        'length': len,
        'append': op.add,
        'list': lambda *x: list(x),

        # is-a
        'null?': lambda x: x == [],
        'symbol?': lambda x: isinstance(x, Symbol),
        'number?': lambda x: isinstance(x, Number),
        'list?': lambda x: isinstance(x, List),
        'procedure?': callable,

        # common
        'apply': apply, 
        'abs': abs,
        'map': map,
        'min': min,
        'max': max,
        'round': round,
        'exit': sys.exit,
        
        # IO
        'display': print,
    }
global_env = Env(get_builtins())

# parser
def tokenize(s):
    ''' convert string into a list of tokens '''
    return s.replace('(', ' ( ').replace(')', ' ) ').split()

def atom(token):
    ''' convert a token into a atom '''
    try:
        return int(token)
    except ValueError:
        try:
            return float(token)
        except ValueError:
            if token == '#t':
                return True
            elif token == '#f':
                return False
            else:
                return Symbol(token)

if __name__ == '__main__':
    pass
