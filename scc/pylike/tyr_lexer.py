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

import re
from collections import namedtuple, OrderedDict
from tyr_error import TyrSyntaxError as LexerError
from tyr_ttt import iteritems

class Token(namedtuple('Token', ['name', 'value', 'lineno', 'column'])):
    def __repr__(self):
        return str(tuple(self))

def decode_str(s):
    regex = re.compile(r'\\(r|n|t|\\|\'|")')
    chars = {
        'r':    '\r',
        'n':    '\n',
        't':    '\t',
        '\\':   '\\',
        '"':    '"',
        "'":    "'",
    }
    def replace(matches):
        char = matches.group(1)[0]
        if char not in chars:
            raise Exception('Unknown escape character {}'.format(char))
        return chars[char]
    return regex.sub(replace, s[1:-1])

def decode_num(s):
    try:
        return int(s)
    except ValueError:
        return float(s)

class Lexer(object):
    rules = [
        ('COMMENT',     r'#.*'),
        ('STRING',      r'"((\\"|[^"])*)"'),
        ('STRING',      r"'((\\'|[^'])*)'"),
        ('NUMBER',      r'\d+\.\d+'),
        ('NUMBER',      r'\d+'),
        ('NAME',        r'[a-zA-Z_]\w*'),
        ('WHITESPACE',  '[ \t]+'),
        ('NEWLINE',     r'\n+'),
        ('OPERATOR',    r'[\+\-\*\/%]'),
        ('OPERATOR',    r'<=|>=|==|!=|<|>'),
        ('OPERATOR',    r'\|\||&&'),
        ('OPERATOR',    r'\.\.\.|\.\.'),
        ('OPERATOR',    '!'),
        ('ASSIGN',      '='),
        ('LPARAM',      r'\('),
        ('RPARAM',      r'\)'),
        ('LBRACK',      r'\['),
        ('RBRACK',      r'\]'),
        ('LCBRACK',     r'{'),
        ('RCBRACK',     r'}'),
        ('COLON',       ':'),
        ('COMMA',       ','),
    ]
    keywords = {
        'func':     'FUNCTION',
        'return':   'RETURN',
        'else':     'ELSE',
        'elif':     'ELIF',
        'if':       'IF',
        'while':    'WHILE',
        'break':    'BREAK',
        'continue': 'CONTINUE',
        'for':      'FOR',
        'in':       'IN',
        'match':    'MATCH',
        'when':     'WHEN',
    }
    ignore_tokens = ['WHITESPACE', 'COMMENT']
    decodes = {'STRING': decode_str, 'NUMBER': decode_num}

    def __init__(self):
        self.source_lines = []
        self.regex = self.compile_rules(self.rules)

    def compile_rules(self, rules):
        return re.compile('|'.join(self.convert_rules(rules)))

    def convert_rules(self, rules):
        grouped_rules = OrderedDict()
        for name, pattern in rules:
            grouped_rules.setdefault(name, [])
            grouped_rules[name].append(pattern)
        for name, patterns in iteritems(grouped_rules):
            joined_patterns = '|'.join(['({})'.format(p) for p in patterns])
            yield '(?P<{}>{})'.format(name, joined_patterns)

    def tokenize_line(self, line, lineno):
        pos = 0
        while pos < len(line):
            matches = self.regex.match(line, pos)
            if matches is not None:
                name = matches.lastgroup
                pos = matches.end(name)
                if name not in self.ignore_tokens:
                    value = matches.group(name)
                    if name in self.decodes:
                        value = self.decodes[name](value)
                    elif name == 'NAME' and value in self.keywords:
                        name = self.keywords[value]
                        value = None
                    yield Token(name, value, lineno, matches.start() + 1)
            else:
                raise LexerError('Unexcepted character {}'.format(line[pos]), lineno, pos + 1)

    def count_leading_characters(self, line, char):
        count = 0
        for c in line:
            if c != char:
                break
            count += 1
        return count

    def detect_indent(self, line):
        if line[0] in [' ', '\t']:
            return line[0] * self.count_leading_characters(line, line[0])

    def tokenize(self, s):
        indent_symbol = None
        tokens = []
        last_indent_level = 0
        lineno = 0
        for lineno, line in enumerate(s.splitlines()):
            lineno += 1
            line = line.rstrip()

            if not line:
                self.source_lines.append('')
                continue

            if indent_symbol is None:
                indent_symbol = self.detect_indent(line)

            if indent_symbol is not None:
                indent_level = line.count(indent_symbol)
                line = line[indent_level * len(indent_symbol):]
            else:
                indent_level = 0

            self.source_lines.append(line)

            line_tokens = list(self.tokenize_line(line, lineno))
            if line_tokens:
                if indent_level != last_indent_level:
                    if indent_level > last_indent_level:
                        tokens.extend([Token('INDENT', None, lineno, 0)] * (indent_level - last_indent_level))
                    elif indent_level < last_indent_level:
                        tokens.extend([Token('DEDENT', None, lineno, 0)] * (last_indent_level - indent_level))
                    last_indent_level = indent_level

                tokens.extend(line_tokens)
                tokens.append(Token('NEWLINE', None, lineno, len(line) + 1))

        if last_indent_level > 0:
            tokens.extend([Token('DEDENT', None, lineno, 0)] * last_indent_level)
        return tokens

class TokenStream(object):
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def consume_expected(self, *args):
        token = None
        for expected_name in args:
            token = self.consume()
            if token.name != expected_name:
                raise LexerError('Expected {}, got {}'.format(expected_name, token.name), token.lineno, token.column)
        return token

    def consume(self):
        token = self.current()
        self.pos += 1
        return token

    def current(self):
        try:
            return self.tokens[self.pos]
        except:
            last_token = self.tokens[-1]
            raise LexerError('Unexcepted end of input', last_token.lineno, last_token.column)

    def expect_end(self):
        if self.pos != len(self.tokens):
            token = self.current()
            raise LexerError('End excepted', token.lineno, token.column)

    def is_end(self):
        return self.pos == len(self.tokens)
