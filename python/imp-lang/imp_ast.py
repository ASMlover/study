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

class BaseAST(object):
    def __eq__(self, other):
        return (isinstance(other, self.__class__)
                and self.__dict__ == other.__dict__)

    def __ne__(self, other):
        return not self.__eq__(other)

class StmtAST(BaseAST):
    pass

class ExprAST(BaseAST):
    pass

class AssignStmtAST(StmtAST):
    def __init__(self, name, expr):
        self.name = name
        self.expr = expr

    def __repr__(self):
        return 'AssignStmtAST(%s, %s)' % (self.name, self.expr)

    def eval(self, env):
        env[self.name] = self.expr.eval(env)

class CompoundStmtAST(StmtAST):
    def __init__(self, first, second):
        self.first = first
        self.second = second

    def __repr__(self):
        return 'CompoundStmtAST(%s, %s)' % (self.first, self.second)

    def eval(self, env):
        self.first.eval(env)
        self.second.eval(env)

class IfStmtAST(StmtAST):
    def __init__(self, cond, true_stmt, else_stmt):
        self.cond = cond
        self.true_stmt = true_stmt
        self.else_stmt = else_stmt

    def __repr__(self):
        return 'IfStmtAST(%s, %s, %s)' % (
                self.cond, self.true_stmt, self.else_stmt)

    def eval(self, env):
        if self.cond.eval(env):
            self.true_stmt.eval(env)
        else:
            self.else_stmt and self.else_stmt.eval(env)

class WhileStmtAST(StmtAST):
    def __init__(self, cond, body):
        self.cond = cond
        self.body = body

    def __repr__(self):
        return 'WhileStmtAST(%s, %s)' % (self.cond, self.body)

    def eval(self, env):
        while self.cond.eval(env):
            self.body.eval(env)

class IntExprAST(ExprAST):
    def __init__(self, value):
        self.value = value

    def __repr__(self):
        return 'IntExprAST(%d)' % self.value

    def eval(self, env):
        return self.value

class VariableExprAST(ExprAST):
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return 'VariableExprAST(%s)' % self.name

    def eval(self, env):
        return env.get(self.name, 0)

class BinaryOperExprAST(ExprAST):
    def __init__(self, op, lexpr, rexpr):
        self.op = op
        self.lexpr = lexpr
        self.rexpr = rexpr

    def __repr__(self):
        return 'BinaryOperExprAST(%s, %s, %s)' % (
                self.op, self.lexpr, self.rexpr)

    def eval(self, env):
        lvalue = self.lexpr.eval(env)
        rvalue = self.rexpr.eval(env)
        if self.op == '+':
            value = lvalue + rvalue
        elif self.op == '-':
            value = lvalue - rvalue
        elif self.op == '*':
            value = lvalue * rvalue
        elif self.op == '/':
            if rvalue == 0:
                raise ZeroDivisionError('%s is zero' % self.rexpr)
            value = lvalue / rvalue
        else:
            raise RuntimeError('Unknown operator: %s' % self.op)
        return value

class LogicOperExprAST(ExprAST):
    def __init__(self, op, lexpr, rexpr):
        self.op = op
        self.lexpr = lexpr
        self.rexpr = rexpr

    def __repr__(self):
        return 'LogicOperExprAST(%s, %s, %s)' % (
                self.op, self.lexpr, self.rexpr)

    def eval(self, env):
        lvalue = self.lexpr.eval(env)
        rvalue = self.rexpr.eval(env)
        if self.op == '<':
            value = lvalue < rvalue
        elif self.op == '<=':
            value = lvalue <= rvalue
        elif self.op == '>':
            value = lvalue > rvalue
        elif self.op == '>=':
            value = lvalue >= rvalue
        elif self.op == '==':
            value = lvalue == rvalue
        elif self.op == '!=':
            value = lvalue != rvalue
        else:
            raise RuntimeError('Unknown operator: %s' % self.op)
        return value

class AndExprAST(ExprAST):
    def __init__(self, lexpr, rexpr):
        self.lexpr = lexpr
        self.rexpr = rexpr

    def __repr__(self):
        return 'AndExprAST(%s, %s)' % (self.lexpr, self.rexpr)

    def eval(self, env):
        return self.lexpr.eval(env) and self.rexpr.eval(env)

class OrExprAST(ExprAST):
    def __init__(self, lexpr, rexpr):
        self.lexpr = lexpr
        self.rexpr = rexpr

    def __repr__(self):
        return 'OrExprAST(%s, %s)' % (self.lexpr, self.rexpr)

    def eval(self, env):
        return self.lexpr.eval(env) or self.rexpr.eval(env)

class NotExprAST(ExprAST):
    def __init__(self, expr):
        self.expr = expr

    def __repr__(self):
        return 'NotExprAST(%s)' % self.expr

    def eval(self, env):
        return not self.expr.eval(env)
