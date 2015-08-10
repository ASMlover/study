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

class Sequence(Eval, Egal):
    def __init__(self):
        self.data = None

    def car(self):
        return self.data[0]

    def cdr(self):
        raise UnimplementedFunctionError(
                'Function not yet implemented for ', 
                self.__class__.__name__)
    def cons(self, e):
        raise UnimplementedFunctionError(
                'Function not yet implemented for ', 
                self.__class__.__name__)

    def __iter__(self):
        return self.data.__iter__()

    def __len__(self):
        return len(self.data)

    def __contains__(self, e):
        return e in self.data

    def __getitem__(self, e):
        return self.data[e]

    def __eq__(self, rhs):
        if not isinstance(rhs, Sequence):
            return False

        if len(self) != len(rhs):
            return False

        for i in range(len(self.data)):
            if self.data[i] != rhs.data[i]:
                return False

        return True

class List(Sequence):
    def __init__(self, l=None):
        super(List, self).__init__()

        self.data = [] if l is None else l

    def cdr(self):
        try:
            return List(self.data[1:])
        except:
            return List([])

    def cons(self, e):
        result = List(self.data[:])
        result.data.insert(0, e)
        return result

    def eval(self, env, args=None):
        form = self.car().eval(env)
        return form.eval(env, self.cdr())

    def __repr__(self):
        if self.data == []:
            return '()'

        result = '(%s' % self.data[0]
        for e in selfd.data[1:]:
            result = '%s %s' % (result, e)
        return result + ')'
