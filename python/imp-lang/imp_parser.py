#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

from functools import reduce
import const
import combinator as cb
import imp_ast as ast

def parse_keyword(kw):
    return cb.Reserved(kw, const.RESERVED)

id = cb.Tag(const.ID)
num = cb.Tag(const.INT) ^ (lambda v: int(v))

def parse(tokens):
    return do_parser()(tokens, 0)

def do_parser():
    return cb.Phrase(stmt_list())

def stmt_list():
    sep = parse_keyword(';') ^ (lambda x: lambda l, r: ast.CompoundStmtAST(l, r))
    return cb.MulParser(stmt(), sep)

def stmt():
    return assign_stmt() | if_stmt() | while_stmt()

def assign_stmt():
    def process(parsed):
        ((name, _), expr) = parsed
        return ast.AssignStmtAST(name, expr)
    return id + parse_keyword('=') + a_expr() ^ process

def if_stmt():
    def process(parsed):
        (((((_, cond), _), true_stmt), else_parsed), _) = parsed
        if else_parsed:
            (_, else_stmt) = else_parsed
        else:
            else_stmt = None
        return ast.IfStmtAST(cond, true_stmt, else_stmt)
    return (parse_keyword('if') + b_expr() +
            parse_keyword('then') + cb.Lazy(stmt_list) +
            cb.Option(parse_keyword('else') + cb.Lazy(stmt_list)) +
            parse_keyword('end') ^ process)

def while_stmt():
    def process(parsed):
        ((((_, cond), _), body), _) = parsed
        return ast.WhileStmtAST(cond, body)
    return (parse_keyword('while') + b_expr() + parse_keyword('do') +
            cb.Lazy(stmt_list) + parse_keyword('end') ^ process)

def b_expr():
    return precedence(b_expr_term(),
            b_expr_precedence_levels, process_logic)

def b_expr_term():
    return b_expr_not() | b_expr_logic() | b_expr_group()

def b_expr_not():
    return (parse_keyword('not') + cb.Lazy(b_expr_term) ^
            (lambda parsed: ast.NotExprAST(parsed[1])))

def b_expr_logic():
    ops = ['<', '<=', '>', '>=', '==', '!=']
    return a_expr() + any_oper_in_list(ops) + a_expr() ^ process_logic_oper

def b_expr_group():
    return (parse_keyword('(') + cb.Lazy(b_expr) +
            parse_keyword(')') ^ process_group)

def a_expr():
    return precedence(a_expr_term(),
            a_expr_precedence_levels, process_binary_oper)

def a_expr_term():
    return parse_expr_value() | a_expr_group()

def a_expr_group():
    return (parse_keyword('(') + cb.Lazy(a_expr) +
            parse_keyword(')') ^ process_group)

def parse_expr_value():
    return ((num ^ (lambda x: ast.IntExprAST(x))) |
            (id ^ (lambda x: ast.VariableExprAST(x))))

def precedence(val_parser, precedence_levels, combine):
    def op_parser(precedence_level):
        return any_oper_in_list(precedence_level) ^ combine
    parser = val_parser * op_parser(precedence_levels[0])
    for precedence_level in precedence_levels[1:]:
        parser = parser * op_parser(precedence_level)
    return parser

def process_binary_oper(op):
    return lambda l, r: ast.BinaryOperExprAST(op, l, r)

def process_logic_oper(parsed):
    ((l, op), r) = parsed
    return ast.LogicOperExprAST(op, l, r)

def process_logic(op):
    if op == 'and':
        return lambda l, r: ast.AndExprAST(l, r)
    elif op == 'or':
        return lambda l, r: ast.OrExprAST(l, r)
    else:
        raise RuntimeError('Unknown logic operator: %s' % op)

def process_group(parsed):
    ((_, p), _) = parsed
    return p

def any_oper_in_list(ops):
    op_parsers = [parse_keyword(op) for op in ops]
    parser = reduce(lambda l, r: l | r, op_parsers)
    return parser

a_expr_precedence_levels = [
    ('*', '/'),
    ('+', '-'),
]

b_expr_precedence_levels = [
    ('and'),
    ('or'),
]
