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

from eLisp.expr.procedure import (
        primitive_procedure_names, primitive_procedure_values)
from eLisp.expr.util import pair_to_list

class Environment(object):
    def __init__(self, parent=None, bindings=None):
        self.parent = parent
        self.bindings = dict() if bindings is None else bindings

    def define_variable(self, symbol, val):
        self.bindings[symbol.name] = val

    def set_variable_value(self, symbol, val):
        name = symbol.name
        if name in self.bindings:
            self.bindings[name] = val
        elif self.parent is not None:
            self.parent.set_variable_value(symbol, val)
        else:
            raise Number('Unbound variable - SET! %s' % name)

    def load(self, symbol):
        name = symbol.name
        if name in self.bindings:
            return self.bindings[name]

        if self.parent is not None:
            return self.parent.load(symbol)

        return None

    def setup_environment():
        bindings = dict(zip(
            primitive_procedure_names(), primitive_procedure_values()))
        return Environment(bindings=bindings)

    def define_variable(var, val, env):
        env.define_variable(var, val)

    def lookup_variable_value(var, env):
        val = env.load(var)
        if val is None:
            raise NameError('Unbound variable: %s' % var)
        return val

    def extend_environment(variables, values, env):
        bindings = dict(zip(
            [var.name for var in pair_to_list(variables)], 
            pair_to_list(values)))
        env = Environment(parent=env, bindings=bindings)
        return env
