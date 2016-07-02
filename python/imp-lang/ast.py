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

class BaseAst(object):
    def __eq__(self, other):
        return (isinstance(other, self.__class__)
                and self.__dict__ == other.__dict__)

    def __ne__(self, other):
        return not self.__eq__(other)

class StmtAst(BaseAst):
    pass

def ExprAst(BaseAst):
    pass

class AssignStmtAst(BaseAst):
    def __init__(self, name, expr):
        self.name = name
        self.expr = expr

    def __repr__(self):
        return 'AssignStmtAst(%s, %s)' % (self.name, self.expr)

    def eval(self, env):
        env[self.name] = self.expr.eval(env)

class CompoundStmtAst(StmtAst):
    def __init__(self, first, second):
        self.first = first
        self.second = second

    def __repr__(self):
        return 'CompoundStmtAst(%s, %s)' % (self.first, self.second)

    def eval(self, env):
        self.first.eval(env)
        self.second.eval(env)

class IfStmtAst(StmtAst):
    def __init__(self, cond, true_stmt, false_stmt):
        self.cond = cond
        self.true_stmt = true_stmt
        self.false_stmt = false_stmt

    def __repr__(self):
        return 'IfStmtAst(%s, %s, %s)' % (
                self.cond, self.true_stmt, self.false_stmt)

    def eval(self, env):
        if self.cond.eval(env):
            self.true_stmt.eval(env)
        else:
            self.false_stmt and self.false_stmt.eval(env)

class WhileStmtAst(StmtAst):
    def __init__(self, cond, body):
        self.cond = cond
        self.body = body

    def __repr__(self):
        return 'WhileStmtAst(%s, %s)' % (self.cond, self.body)

    def eval(self, env):
        while self.cond.eval(env):
            self.body.eval(env)

class IntExprAst(ExprAst):
    def __init__(self, v):
        self.v = v

    def __repr__(self):
        return 'IntExprAst(%d)' % self.v

    def eval(self, env):
        return self.v

class VariableExprAst(ExprAst):
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return 'VariableExprAst(%s)' % self.name

    def eval(self, env):
        return env.get(self.name, 0)

class BinOperExprAst(ExprAst):
    def __init__(self, op, left, right):
        self.op = op
        self.left = left
        self.right = right

    def __repr__(self):
        return 'BinOperExprAst(%s, %s, %s)' % (
                self.op, self.left, self.right)

    def eval(self, env):
        lval = self.left.eval(env)
        rval = self.right.eval(env)
        if self.op == '+':
            val = lval + rval
        elif self.op == '-':
            val = lval - rval
        elif self.op == '*':
            val = lval * rval
        elif self.op == '/':
            val = lval / rval
        else:
            raise RuntimeError('Unknown operator: %s' % self.op)
        return val

class LogicExprAst(ExprAst):
    def __init__(self, op, left, right):
        self.op = op
        self.left = left
        self.right = right

    def __repr__(self):
        return 'LogicExprAst(%s, %s, %s)' % (self.op, self.left, self.right)

    def eval(self, env):
        lval = self.left.eval(env)
        rval = self.right.eval(env)
        if self.op == '<':
            val = lval < rval
        elif self.op == '<=':
            val = lval <= rval
        elif self.op == '>':
            val = lval > rval
        elif self.op == '>=':
            val = lval >= rval
        elif self.op == '==':
            val = lval == rval
        elif self.op == '!=':
            val = lval != rval
        else:
            raise RuntimeError('Unknown operator: %s' % self.op)
        return val

class AndExprAst(ExprAst):
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def __repr__(self):
        return 'AndExprAst(%s, %s)' % (self.left, self.right)

    def eval(self, env):
        return self.left.eval(env) and self.right.eval(env)

class OrExprAst(ExprAst):
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def __repr__(self):
        return 'OrExprAst(%s, %s)' % (self.left, self.right)

    def eval(self, env):
        return self.left.eval(env) or self.right.eval(env)

class NotExprAst(ExprAst):
    def __init__(self, expr):
        self.expr = expr

    def __repr__(self):
        return 'NotExprAst(%s)' % self.expr

    def eval(self, env):
        return not self.expr.eval(env)
