#!/usr/bin/env python
# -*- encoding: utf-8 -*-
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

import operator
from collections import namedtuple
import tyr_ast as ast
from tyr_lexer import Lexer, TokenStream
from tyr_parser import Parser
from tyr_error import TyrSyntaxError, report_syntax_error
from tyr_utils import print_ast, print_tokens, print_env

BuiltinFunction = namedtuple('BuiltinFunction', ['params', 'body'])

class Break(Exception):
    pass

class Continue(Exception):
    pass

class Return(Exception):
    def __init__(self, value):
        self.value = value

class Environment(object):
    def __init__(self, parent=None, args=None):
        self.parent = parent
        self.values = {}
        if args:
            self.init_from_dict(args)

    def init_from_dict(self, args):
        for key, value in args.items():
            self.set(key, value)

    def set(self, key, value):
        self.values[key] = value


    def get(self, key):
        value = self.values.get(key, None)
        if value is None and self.parent:
            return self.parent.get(key)
        else:
            return value

    def as_dict(self):
        return self.values

    def __repr__(self):
        return 'Environment({})'.format(str(self.values))
