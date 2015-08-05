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

import itertools

from eLisp import tokens
from eLisp.model import (
        Number, Boolean, Character, String, Symbol, Pair, EmptyList)

class ParserException(Exception):
    pass

class Parser(object):
    """ Parser for a scheme subset """
    def __init__(self, lexer):
        self.lexer = lexer
        self.marks = []
        self.lookhead = []
        self.pos = 0
        self._sync(1)

    def parse(self):
        result = []
        while self._lookahead_type(0) != tokens.EOF:
            result.append(self._datum())
        return result

    def _datum(self):
        if self._lookahead_type(0) in (tokens.LPAREN, tokens.QUOTE):
            return self._list()
        return self._simple_datum()

    def _simple_datum(self):
        token = self._lookahead_token(0)

        if token.type == tokens.NUMBER:
            expr = Number(int(token.text))
        elif token.type == tokens.BOOLEAN:
            expr = Boolean(True if token.text == '#t' else False)
        elif token.type == tokens.CHARACTER:
            expr = Character(token.text[2:])
        elif token.type == tokens.STRING:
            expr = String(token.text.strip('"'))
        elif token.type == tokens.ID:
            expr = Symbol(token.text)
        else:
            raise ParserException('No viable alternative')

        self._match(token.type)
        return expr

    def _abbreviation(self):
        self._match(tokens.QUOTE)

        if self._lookahead_type(0) == tokens.LPAREN:
            expr = self._list()
        else:
            expr = self._simple_datum()

        return Pair(Symbol('quote'), Pair(expr, EmptyList))

    def _list(self):
        result = []
        if self._lookahead_type(0) == tokens.QUOTE:
            return self._abbreviation()
        self._match(tokens.LPAREN)

        if self._lookahead_type(0) == tokens.RPAREN:
            self._match(tokens.RPAREN)
            return EmptyList

        index, dot_index = 0, -1
        while self._lookahead_type(0) != tokens.RPAREN:
            head = self._datum()
            if self._lookahead_type(0) == tokens.DOT:
                self._match(tokens.DOT)
                tail = self._datum()
                result.append(Pair(head, tail))
                dot_index = index
                break
            else:
                result.append(head)
            index += 1
        
        self._match(tokens.RPAREN)
        if dot_index > 0:
            dot_index = len(result) - dot_index - 1
        tail = EmptyList
        for index, expr in enumerate(reversed(result)):
            if index == dot_index:
                tail = expr
            else:
                tail = Pair(expr, tail)
        
        return tail

    # helper methods
    def _sync(self, index):
        if index + self.pos > len(self.lookhead):
            number = index + self.pos - len(self.lookhead)
            self._fill(number)

    def _fill(self, number):
        self.lookhead.extend(itertools.islice(self.lexer, 0, number))

    def _lookahead_type(self, number):
        return self._lookahead_token(number).type

    def _lookahead_token(self, number):
        self._sync(number)
        return self.lookhead[self.pos + number]

    def _match(self, token_type):
        if self._lookahead_type(0) == token_type:
            self._consume()
        else:
            raise ParserException('Excepting %s; found %s' % 
                    token_type, self._lookahead_type(0))

    def _consume(self):
        self.pos += 1
        if self.pos == len(self.lookhead) and not self._is_speculating():
            self.lookhead = []
            self.pos = 0
        self._sync(1)

    def _is_speculating(self):
        return bool(self.marks)

    def _mark(self):
        self.marks.append(self.pos)

    def _release(self):
        self._seek(self.marks.pop())

    def _index(self):
        return self.pos

    def _seek(self, index):
        self.pos = index
