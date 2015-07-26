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

import ast
import copy
import token

class Parser(object):
    def __init__(self, lexer):
        self.lexer = lexer
        self.paren_depth = 0

    def _list_pop(self):
        if len(self.list) > 0:
            return self.list.pop()
        else:
            return None

    def parse(self):
        self.root = ast.Ast(None)
        self.list = self.lexer.lex()
        self.list.reserve()
        while len(self.list) > 0:
            self.parse_list(self.root)
        return self.root

    def parse_list(self, tree):
        t = self._list_pop()
        if t == None:
            return
        type_str = token.TOKEN_TYPES[t.type.type]
        if type_str == 'bparen':
            self.paren_depth += 1
            return self.parse_in_list(tree.add(ast.Ast(token.Token('list', pos=t.pos))))
        elif type_str == 'eparen':
            self.paren_depth -= 1
            if self.paren_depth < 0:
                raise Exception('unmatched paren')
            elif self.paren_depth > 0:
                return self.parse_in_list(tree.get_parent())
            else:
                return self.parse_list(tree.get_parent())
        else:
            raise Exception('unrecognized token')

    def parse_in_list(self, tree):
        while True:
            t = self._list_pop()
            if t == None:
                raise Exception('unexpected end of file')
            type_str = token.TOKEN_TYPES[t.type.type]
            if type_str in ['bparen', 'eparen']:
                self.list.append(t)
                return self.parse_list(tree)
            elif type_str in ['id', 'number', 'str']:
                if type_str == 'number':
                    t.string = int(t.string)
                tree.add(ast.Ast(t))
            else:
                raise Exception('unrecognized token')
