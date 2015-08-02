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

from eLisp.model import Symbol, Boolean, EmptyList
from eLisp.expr.sequence import sequence_expr
from eLisp.expr.util import (
        is_tagged_list, car, cdr, cadr, caddr, cdddr, cadddr, tolist)

def is_if(expr):
    return is_tagged_list(expr, Symbol('if'))

def if_predicate(expr):
    return cadr(expr)

def if_consequent(expr):
    return caddr(expr)

def if_alternative(expr):
    if cdddr(expr) is not EmptyList:
        return cadddr(expr)
    return False

def make_if(predicate, consequent, alternative):
    return tolist(Symbol('if'), predicate, consequent, alternative)

def is_and(expr):
    return is_tagged_list(expr, Symbol('and'))

def is_or(expr):
    return is_tagged_list(expr, Symbol('or'))

def is_cond(expr):
    return is_tagged_list(expr, Symbol('cond'))

def cond_clauses(expr):
    return cdr(expr)

def is_cond_else_clause(clause):
    return cond_predicate(clause) == Symbol('else')

def cond_predicate(clause):
    return car(clause)

def cond_actions(clause):
    return cdr(clause)

def cond_to_if(expr):
    return expand_clauses(cond_clauses(expr))

def expand_clauses(clauses):
    if clauses is EmptyList:
        return Boolean(False)

    first = car(clauses)
    rest = cdr(clauses)

    if is_cond_else_clause(first):
        if rest is EmptyList:
            return sequence_expr(cond_actions(first))
        else:
            raise ValueError('ELSE caluse isn\'t last: %s' % clauses)
    else:
        return make_if(cond_predicate(first), 
                sequence_expr(cond_actions(first)), expand_clauses(rest))
