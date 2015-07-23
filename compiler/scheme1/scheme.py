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

from argparse import ArgumentParser, FileType
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
    if sys.version[0] == '2':
        return raw_input(*args, **kwargs)
    else:
        return input(*args, **kwargs)

def tostring(expr):
    if isinstance(expr, List):
        return '(' + ' '.join(map(tostring, expr)) + ')'
    else:
        return str(expr)

# environment
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

def expr_from_tokens(tokens):
    '''
    pop a complete expression from the start of tokens, and return it.
    '''
    if len(tokens) == 0:
        return SyntaxError('unexpected EOF at start of expression')
    token = tokens.pop(0)
    if token == '(':
        expr = []
        while tokens and tokens[0] != ')':
            expr.append(expr_from_tokens(tokens))
        if len(tokens) == 0:
            raise SyntaxError('unexpected EOF mid expression')
        tokens.pop(0)
        return expr
    elif token == ')':
        raise SyntaxError('unexpected )')
    else:
        return atom(token)

def parse(s):
    ''' yield a sequence of scheme expressions from a string '''
    tokens = tokenize(s)
    while tokens:
        yield expr_from_tokens(tokens)

# eval
def eval_expr(expr, env=global_env):
    ''' evaluate an expression in an environment. '''
    if isinstance(expr, Symbol):
        define_env = env.find(expr)
        if define_env:
            return define_env[expr]
        else:
            raise NameError(expr)
    elif not isinstance(expr, List):
        return expr
    elif expr[0] == 'quote':
        # (quote <value>)
        (_, value) = expr
        return value
    elif expr[0] == 'if':
        # (if <condition> <consequent> <alternate>)
        (_, cond, conseq, alt) = expr
        return eval_expr((conseq if eval_expr(cond, env) else alt), env)
    elif expr[0] == 'set!':
        # (set! <name> <value>)
        (_, name, value) = expr
        env.find(name)[name] = eval_expr(value, env)
    elif expr[0] == 'define':
        # (define <name> <value>)
        (_, name, value) = expr
        env[name] = eval_expr(value, env)
    elif expr[0] == 'lambda':
        # (lambda (<arg> [<arg> ...]) <body>) or (lambda <arg> <body>)
        (_, args, body) = expr
        return lambda *params: eval_expr(body, Env(zip(args, params), env))
    elif expr[0] == 'begin':
        # (begin <expr> [<expr> ...])
        if len(expr) <= 1:
            raise SyntaxError('<begin> with no contents')
        for exp in expr[1:]:
            val = eval_expr(exp, env)
        return val
    else:
        # procedure invocation (<proc> arg1 arg2 ...)
        values = [eval_expr(exp, env) for exp in expr]
        proc = values.pop(0)
        if callable(proc):
            return proc(*values)
        else:
            raise TypeError('expression <%s> (%s) not callable in <%s>'
                    % (expr[0], type(proc).__name__, tostring(expr)))

def eval_string(s, env=global_env):
    value = None
    for expr in parse(s):
        value = eval_expr(expr, env)
    return value

# repl
def repl(prompt='>>> '):
    while True:
        try:
            value = eval_string(get_input(prompt))
        except EOFError:
            break
        if value is not None:
            print(tostring(value))
    print()

# command-line processing
def get_parser():
    parser = ArgumentParser(
            description='load data into the rangespan catalog')
    parser.add_argument('-v', '--version', 
            action='store_true',
            help='show version number and exit.')
    parser.add_argument('sourcefile', 
            nargs='?',
            type=FileType('rU'),
            default=sys.stdin,
            help='scheme source code filename')
    return parser

if __name__ == '__main__':
    parser = get_parser()
    args = parser.parse_args()

    if args.version:
        print ('version-0.1')
        sys.exit(0)

    if sys.stdin.isatty() and args.sourcefile == sys.stdin:
        repl()
    else:
        eval_string(args.sourcefile.read())
