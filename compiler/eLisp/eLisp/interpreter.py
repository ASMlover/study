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

import sys
from eLisp.lexer import Lexer
from eLisp.parser import Parser
from eLisp.model import Symbol, Boolean, EmptyList
from eLisp.scope import (
        setup_environment, define_variable, 
        lookup_variable_value, extend_environment)
from eLisp.expr import (
        selfeval, quote, definition, variable, assignment, 
        conditional, lambdaexpr, procedure, sequence, binding)
from eLisp.expr.util import cons, is_load, load

class Interpreter(object):
    def __init__(self):
        self.env = setup_environment()

    def interpret(self, expr):
        return self._eval(expr, self.env)

    def _eval(self, expr, env):
        if selfeval.is_self_evaluating(expr):
            return expr
        elif quote.is_quoted(expr):
            return quote.text_of_quotation(expr)
        elif definition.is_definition(expr):
            return self._eval_definition(expr, env)
        elif sequence.is_begin(expr):
            return self._eval_sequence(sequence.begin_actions(expr), env)
        elif binding.is_let_binding(expr):
            return self._eval_binding(expr, env)
        elif variable.is_variable(expr):
            return lookup_variable_value(expr, env)
        elif assignment.is_assignment(expr):
            return self._eval_assignment(expr, env)
        elif conditional.is_if(expr):
            return self._eval_if(expr, env)
        elif conditional.is_cond(expr):
            return self._eval(conditional.cond_to_if(expr), env)
        elif conditional.is_and(expr):
            return self._eval_and(expr, env)
        elif conditional.is_or(expr):
            return self._eval_or(expr, env)
        elif lambdaexpr.is_lambda(expr):
            return procedure.make_procedure(
                    lambdaexpr.lambda_parameters(expr), 
                    lambdaexpr.lambda_body(expr), 
                    env)
        elif procedure.is_application(expr):
            return self._apply(
                    self._eval(procedure.operator(expr), env), 
                    self._list_of_values(procedure.operands(expr), env))

    def _eval_definition(self, expr, env):
        define_variable(
                definition.definition_variable(expr), 
                self._eval(definition.definition_value(expr), env), 
                env)
        return Symbol('ok')

    def _eval_assignment(self, expr, env):
        env.set_variable_value(
                assignment.assignment_variable(expr), 
                self._eval(assignment.assignment_value(expr), env))
        return Symbol('ok')

    def _eval_binding(self, expr, env):
        return self._eval(
                cons(lambdaexpr.make_lambda(binding.binding_variables(expr), 
                        binding.binding_body(expr)),
                    binding.binding_values(expr)), 
                env)

    def _eval_if(self, expr, env):
        if self._eval(conditional.if_predicate(expr), env):
            return self._eval(conditional.if_consequent(expr), env)
        return self._eval(conditional.if_alternative(expr), env)

    def _eval_and(self, expr, env):
        if procedure.no_operands(procedure.operands(expr)):
            return Boolean(True)

        def inner(expr, env):
            if sequence.is_last_expr(expr):
                return self._eval(sequence.first_expr(expr), env)
            if not self._eval(sequence.first_expr(expr), env):
                return Boolean(False)
            return inner(sequence.rest_exprs(expr), env)

        return inner(sequence.rest_exprs(expr), env)

    def _eval_or(self, expr, env):
        if procedure.no_operands(procedure.operands(expr)):
            return Boolean(False)

        def inner(expr, env):
            if sequence.is_last_expr(expr):
                return self._eval(sequence.first_expr(expr), env)
            first_value = self._eval(sequence.first_expr(expr), env)
            if first_value:
                return first_value
            return inner(sequence.rest_exprs(expr), env)

        return inner(sequence.rest_exprs(expr), env)

    def _eval_sequence(self, expressions, env):
        if sequence.is_last_expr(expressions):
            return self._eval(sequence.first_expr(expressions), env)
        else:
            self._eval(sequence.first_expr(expressions), env)
            return self._eval_sequence(sequence.rest_exprs(expressions), env)

    def _list_of_values(self, expressions, env):
        if procedure.no_operands(expressions):
            return EmptyList
        else:
            return cons(
                    self._eval(procedure.first_operand(expressions), env), 
                    self._list_of_values(procedure.rest_operands(expressions), env))

    def _apply(self, proc, args):
        if procedure.is_primitive_procedure(proc):
            return procedure.apply_primitive_procedure(proc, args)
        elif procedure.is_compound_procedure(proc):
            return self._eval_sequence(
                    procedure.procedure_body(proc), 
                    extend_environment(
                        procedure.procedure_parameters(proc), 
                        args, 
                        procedure.procedure_environment(proc)))

def get_input(*args):
    if sys.version[0] == '2':
        return raw_input(*args)
    else:
        return input(*args)

def main():
    interpreter = Interpreter()
    while True:
        try:
            buffer = get_input('eLisp> ')
        except EOFError:
            print()
            break
        lexer = Lexer(buffer)

        try:
            exprs = Parser(lexer).parse()
            if not exprs:
                continue
            expr = exprs[0]

            if is_load(expr):
                load(interpreter, expr)
                continue

            result_expr = interpreter.interpret(expr)
            if procedure.is_compound_procedure(result_expr):
                print (procedure.get_procedure_repr(result_expr))
            else:
                print (result_expr)
        except Exception as e:
            print (str(e))
            continue
