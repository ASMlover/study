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
import collections import namedtuple, OrderedDict
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
        '"':    '"'
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
                    elif name == 'NAME' and value in self,.keywords:
                        name = self.keywords[value]
                        value = None
                    yield Token(name, value, lineno, matches.start() + 1)
            else:
                raise LexerError('Unexcepted character {}'.format(line[pos]), lineno, pos + 1)
