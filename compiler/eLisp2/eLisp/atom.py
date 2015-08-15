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

from error import UnimplementedFunctionError
from interface import Eval, Egal
from seq import Sequence, List

class Atom(Eval, Egal):
    def __init__(self, data):
        self.data = data
        self.hint = 'atom'

    def __eq__(self, rhs):
        if isinstance(rhs, Atom):
            return self.data == rhs.data
        else:
            return False

class Symbol(Atom):
    def __init__(self, symbol):
        super(Symbol, self).__init__(symbol)

    def __repr__(self):
        return self.data

    def __hash__(self):
        return hash(self.data)

    def eval(self, env, args=None):
        return env.get(self.data)

TRUE = Symbol('#t')
FALSE = List()

class String(Atom, Sequence):
    def __init__(self, str):
        Atom.__init__(self, str)

    def __repr__(self):
        return repr(self.data)

    def eval(self, env, args=None):
        return self

    def cons(self, e):
        if e.__class__ != self.__class__ and e.__class__ != Symbol.__class__:
            raise UnimplementedFunctionError(
                    'Cannot cons a string and a ', 
                    e.__class__.__name__)
        return String(e.data + self.data)

    def car(self):
        """
        `car` is roughly the same as `first` in linear eLisp
        """
        return Symbol(self.data[0])

    def cdr(self):
        """
        `cdr` is roughly the same as 'rest' in linear eLisp
        """
        return String(self.data[1:])
