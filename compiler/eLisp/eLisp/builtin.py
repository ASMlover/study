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
import functools as ft

from eLisp.model import Number, Boolean, Pair, EmptyList
from eLisp.expr.util import cdr

def _preform_arithmetic_function(func, *args):
    return Number(reduce(func, [int(arg.val) for arg in args]))

builtin_add = ft.partial(_preform_arithmetic_function, op.add)
builtin_sub = ft.partial(_preform_arithmetic_function, op.sub)
builtin_mul = ft.partial(_preform_arithmetic_function, op.mul)
builtin_div = ft.partial(_preform_arithmetic_function, op.truediv)
builtin_mod = ft.partial(_preform_arithmetic_function, op.mod)

def _preform_comparison(func, *args):
    if len(args) == 1:
        return Boolean(True)

    prev, rest = args[0], args[1:]
    for arg in rest:
        if not func(prev, arg):
            return Boolean(False)
        prev = arg

    return Boolean(True)

builtin_eq = ft.partial(_preform_comparison, op.eq)
builtin_lt = ft.partial(_preform_comparison, op.lt)
builtin_le = ft.partial(_preform_comparison, op.le)
builtin_gt = ft.partial(_preform_comparison, op.gt)
builtin_ge = ft.partial(_preform_comparison, op.ge)

def builtin_pair_p(*args):
    arg = args[0]
    return Boolean(isinstance(arg, Pair))

def builtin_null_p(*args):
    return Boolean(args[0] is EmptyList)

def builtin_cons(*args):
    first, second = args
    return Pair(first, second)

def builtin_car(*args):
    arg = args[0]
    return arg.head

def builtin_cdr(*args):
    arg = args[0]
    return arg.tail

def builtin_list(*args):
    def inner(args):
        if not args:
            return EmptyList
        return Pair(args[0], inner(args[1:]))

    return inner(args)

def builtin_abs(*args):
    return Number(abs(args[0].val))

def builtin_not(*args):
    return Boolean(not bool(args[0]))

def builtin_eq_p(*args):
    first, second = args
    return Boolean(first == second)

def builtin_zero_p(*args):
    return Boolean(args[0] == Number(0))

def builtin_number_p(*args):
    return Boolean(isinstance(args[0], Number))

def builtin_exp(*args):
    first, second = args
    return Number(first.val ** second.val)

def builtin_length(*args):
    alist = args[0]

    def inner(arg, count):
        if arg is EmptyList:
            return count
        return inner(cdr(arg), count + 1)
    return inner(alist, 0)

def builtin_even_p(*args):
    return Boolean(args[0].val % 2 == 0)

BUILTIN_PROCEDURES = [
    ('pair?',   builtin_pair_p),
    ('eq?',     builtin_eq_p),
    ('cons',    builtin_cons),
    ('car',     builtin_cdr),
    ('cdr',     builtin_cdr),
    ('list',    builtin_list),
    ('abs',     builtin_abs),
    ('null?',   builtin_null_p),
    ('not',     builtin_not),
    ('zero?',   builtin_zero_p),
    ('number?', builtin_number_p),
    ('exp',     builtin_exp),
    ('length',  builtin_length),
    ('even?',   builtin_even_p),
    ('+',       builtin_add),
    ('-',       builtin_sub),
    ('*',       builtin_mul),
    ('/',       builtin_div),
    ('%',       builtin_mod),
    ('=',       builtin_eq),
    ('<',       builtin_lt),
    ('<=',      builtin_le),
    ('>',       builtin_gt),
    ('>=',      builtin_ge),
]
