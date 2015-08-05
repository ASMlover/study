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
from eLisp import tokens

class Token(object):
    def __init__(self, type, text):
        self.type = type
        self.text = text

    def __str__(self):
        return '<\'{text}\', {name}>'.format(text=self.text, name=self.type)

class LexerException(Exception):
    def __init__(self, msg, pos):
        self.msg = msg
        self.pos = pos

    def __str__(self):
        return 'Error at position: %s - %s' % (self.pos, self.msg)

class Lexer(object):
    RULES = [
        (r'[+-]?\d+(?![a-zA-Z])', tokens.NUMBER),
        (r'#t|#f', tokens.BOOLEAN),
        (r'#\\(?:newline|space|[a-zA-Z])', tokens.CHARACTER),
        (r'".*"', tokens.STRING),
        (r'\(', tokens.LPAREN),
        (r'\)', tokens.RPAREN),
        (r'\'', tokens.QUOTE),
        (r'(?<=\s)\.(?=\s)', tokens.DOT),
        (r'[+-](?!\w)|[<>]=|[=*/><]?\d*([a-zA-Z_]*\w*[!$%&*/:<=>?^_~+-.@]*)*', tokens.ID),
    ]
    IS_WHITESPACE   = re.compile(r'\s+').match
    IS_COMMENT      = re.compile(r';.*').match

    def __init__(self, buffer):
        self.buffer = buffer
        self.pos = 0
        self.regexp = self._build_master_regexp()

    def _build_master_regexp(self):
        result = []
        for regexp, group_name in self.RULES:
            result.append(r'(?P<%s>%s)' % (group_name, regexp))
        master_regexp = re.compile('|'.join(result), re.MULTILINE)
        return master_regexp

    def token(self):
        buffer, regexp = self.buffer, self.regexp
        IS_WHITESPACE = self.IS_WHITESPACE
        IS_COMMENT = self.IS_COMMENT
        end = len(buffer)

        while True:
            match = (IS_WHITESPACE(buffer, self.pos) or 
                    IS_COMMENT(buffer, self.pos))
            if match is not None:
                self.pos = match.end()
            else:
                break

        if self.pos >= end:
            return Token(tokens.EOF, 'EOF')

        match = regexp.match(buffer, self.pos)
        if match is None:
            raise LexerException('No valid token', self.pos)

        self.pos = match.end()

        group_name = match.lastgroup
        token = Token(group_name, match.group(group_name))
        return token

    def __iter__(self):
        return self.next()

    def next(self):
        while True:
            token = self.token()
            if token.type == tokens.EOF:
                yield token
                return
            yield token
