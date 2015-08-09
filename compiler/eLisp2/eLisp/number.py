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

import re
import types
from interface import Eval

class Number(Eval):
    def __init__(self, value):
        self.data = value

    def __repr__(self):
        return repr(self.data)

    def eval(self, env, args=None):
        return self

    def __eq__(self, rhs):
        if isinstance(rhs, Number):
            return (self.data == rhs.data)
        else:
            return False

class Integral(Number):
    REGEX = re.compile(r'^[+-]?\d+$')

    def __init__(self, value):
        super(Integral, self).__init__(value)

class LongInt(Number):
    REGEX = re.compile(r'^[+-]?\d+[lL]$')

    def __init__(self, value):
        super(LongInt, self).__init__(value)

class Float(Number):
    REGEX = re.compile(r'^[+-]?(\d+\.\d*$|\d*\.\d+$)')

    def __init__(self, value):
        super(Float, self).__init__(value)
