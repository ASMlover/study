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

from atom import FALSE
from interface import Eval

class Function(Eval):
    def __init__(self, fun):
        self.fun = fun
        self.hint = 'fun'

    def __repr__(self):
        return '<built-in function %s>' % id(self.fun)

    def eval(self, env, args):
        """
        Evaluation just delegates out to the builtin.
        """
        return self.fun(env, args)

class Lambda(Eval):
    def __init__(self, names, body):
        self.names = names
        self.body = body
    
    def __repr__(self):
        return '<lambda %s>' % id(self)

    def set_bindings(self, containing_env, values):
        """
        The bindings are set one by one corresponding to 
        the input values.
        """
        for i in range(len(values)):
            containing_env.environment.binds[self.names[i].data] = 
                values[i].eval(containing_env.env)

    def push_bindings(self, containing_env, values):
        containing_env.push()
        self.set_bindings(containing_env, values)

    def eval(self, env, args):
        values = [a for a in args]
        
        if len(values) != len(self.names):
            raise ValueError(
                'Wrong number of arguments, expect {0}, got {1}'.format(
                    len(self.names), len(args)))
        
        ELISP = env.get('__elisp__')
        self.push_bindings(ELISP, values)
        
        result = FALSE
        for form in self.body:
            result = form.eval(ELISP.environment)
        
        ELISP.pop()
        return result

class Closure(Lambda):
    def __init__(self, env, names, body):
        super(Closure, self).__init__(names, body)
        self.env = env

    def __repr__(self):
        return '<lexical closure %s>' % int(self)

    def push_bindings(self, containing_env, values):
        containing_env.push(self.env.binds)
        self.set_bindings(containing_env, values)
