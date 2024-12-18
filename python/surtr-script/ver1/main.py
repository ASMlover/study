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

from __future__ import print_function
import re

TYPE_INVALID = -1
TYPE_EOF = 0
TYPE_STRING = 1
TYPE_IDENTIFIER = 2
TYPE_LPAREN = 3 # (
TYPE_RPAREN = 4 # )
TYPE_COMMENT = 5 # comment begin with `#`
TYPE_COMMA = 6 # `,`

TYPE_NAME_MAPS = {
    TYPE_INVALID: '<invalid>',
    TYPE_EOF: '<eof>',
    TYPE_STRING: '<string>',
    TYPE_IDENTIFIER: '<identifier>',
    TYPE_LPAREN: '<left paren>',
    TYPE_RPAREN: '<right paren>',
    TYPE_COMMA: '<comma>',
}

RESERVED_MAPS = {
}

class Token(object):
    def __init__(self, text, type=TYPE_INVALID, lineno=0):
        self.text = text
        self.type = type
        self.lineno = lineno

    def __repr__(self):
        return '{name} - @text: "{text}", @type: {type}, @lineno: {lineno}'.format(
                    name=TYPE_NAME_MAPS.get(self.type, '<invalid>'),
                    text=self.text,
                    type=self.type,
                    lineno=self.lineno
                )

class Lexer(object):
    def __init__(self, fname):
        self.content = open(fname, mode='r', encoding='utf-8').read()
        self.pos = 0
        self.lineno = 1

    def get_char(self):
        if self.pos >= len(self.content):
            return None

        c = self.content[self.pos]
        self.pos += 1
        return c

    def unget_char(self):
        self.pos -= 1

    def get_string(self):
        text_list = []
        while True:
            c = self.get_char()
            if c is None or c == '\r' or c == '\n' or c == '\\':
                raise Exception('invalid source string at line: %d' % self.lineno)
            elif c == '"':
                break
            else:
                text_list.append(c)
        return Token(''.join(text_list), TYPE_STRING, self.lineno)

    def get_identifier(self):
        self.unget_char()

        regex = re.compile('[a-z_A-Z][a-z_A-Z0-9]*')
        match = regex.match(self.content, self.pos)
        if match:
            text = match.group(0)
            self.pos = match.end(0)
        else:
            raise Exception('invalid identifier source at line: %s' % self.lineno)

        return Token(text, TYPE_IDENTIFIER, self.lineno)

    def skip_comment(self):
        while True:
            c = self.get_char()
            if c is None:
                break
            elif c == '\r' or c == '\n':
                self.lineno += 1
                break

    def get_token(self):
        while True:
            c = self.get_char()

            if c is None:
                return None
            elif c == '\n':
                self.lineno += 1
            elif c == '\r':
                if self.get_char() != '\n':
                    self.unget_char()
                self.lineno += 1
            elif c == ' ' or c == '\t':
                pass
            elif str.isalpha(c) or c == '_':
                return self.get_identifier()
            elif c == '#':
                self.skip_comment()
            elif c == '"':
                return self.get_string()
            elif c == '(':
                return Token('(', TYPE_LPAREN, self.lineno)
            elif c == ')':
                return Token(')', TYPE_RPAREN, self.lineno)
            elif c == ',':
                return Token(',', TYPE_COMMA, self.lineno)

class Functor(object):
    def __init__(self, fn, name, nargs):
        self.closure = fn
        self.name = name
        self.nargs = nargs

    def __call__(self, args):
        self.closure(*args)

class Executor(object):
    def __init__(self):
        self.init_env()

    def init_env(self):
        self._env = {
            'show_msg': Functor(self._show_msg, '_show_msg', 1)
        }

    def _show_msg(self, msg):
        print(msg)

    def do_funcall(self, lex, fn):
        args = []
        while True:
            tok = lex.get_token()
            if tok.type == TYPE_RPAREN:
                break
            elif tok.type == TYPE_COMMA:
                if len(args) == 0:
                    raise Exception('invalid function call argument with `%s` at %d' % (tok.text, tok.lineno))
            elif tok.type == TYPE_STRING:
                args.append(tok.text)
            else:
                raise Exception('error function arguments')

        if fn.text not in self._env:
            raise Exception('`%s` undefined, at line: %d' % (fn.text, fn.lineno))

        functor = self._env[fn.text]
        if functor.nargs != len(args):
            raise Exception('`%s` arguments count unmatched, at line: %d' % (fn.text, fn.lineno))
        functor(args)

    def do_execute(self, lex, token):
        if token.type == TYPE_IDENTIFIER:
            # parser function call
            next_tok = lex.get_token()
            if next_tok.type == TYPE_LPAREN:
                self.do_funcall(lex, token)
        else:
            raise Exception('error syntx at line: %d' % token.lineno)

    def run(self, fname):
        lex = Lexer(fname)
        while True:
            tok = lex.get_token()
            if tok is None:
                break
            self.do_execute(lex, tok)

def main():
    ex = Executor()
    ex.run('surtr.su')

if __name__ == '__main__':
    main()
