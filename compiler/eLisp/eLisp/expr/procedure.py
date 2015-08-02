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

from eLisp.model import Symbol, Pair, EmptyList
from eLisp.expr.util import (
        is_tagged_list, pair_to_list, car, cdr, cons, cadr, caddr, cadddr)
from eLisp.builtin import BUILTIN_PROCEDURES

def is_application(expr):
    return isinstance(expr, Pair)

def operator(expr):
    return car(expr)

def operands(expr):
    return cdr(expr)

def no_operands(ops):
    return ops is EmptyList

def first_operand(ops):
    return car(ops)

def rest_operands(ops):
    return cdr(ops)

def make_procedure(params, body, env):
    return cons(Symbol('procedure'), 
            cons(params, cons(body, cons(env, EmptyList))))

def is_compound_procedure(expr):
    return is_tagged_list(expr, Symbol('procedure'))

def procedure_parameters(expr):
    return cadr(expr)

def procedure_body(expr):
    return caddr(expr)

def procedure_environment(expr):
    return cadddr(expr)

def get_procedure_repr(expr):
    return '#<procedure %s %s <procedure-env>' % (
            procedure_parameters(expr), procedure_body(expr))

def is_primitive_procedure(expr):
    return is_tagged_list(expr, Symbol('primitive'))

def primitive_implementation(expr):
    return cadr(expr)

def primitive_procedure_names():
    return [name for name, _ in BUILTIN_PROCEDURES]

def primitive_procedure_values():
    return [Pair(Symbol('primitive'), Pair(proc, EmptyList)) 
            for _, proc in BUILTIN_PROCEDURES]

def apply_primitive_procedure(proc, args):
    func = primitive_implementation(proc)
    return func(*pair_to_list(args))
