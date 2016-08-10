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

class Result(object):
    def __init__(self, value, pos):
        self.value = value
        self.pos = pos

    def __repr__(self):
        return 'Result(%s, %d)' % (self.value, self.pos)

class Parser(object):
    def __call__(self, tokens, pos):
        return None

    def __add__(self, other):
        return AddParser(self, other)

    def __mul__(self, other):
        return MulParser(self, other)

    def __or__(self, other):
        return OrParser(self, other)

    def __xor__(self, func):
        return Process(self, func)

class Tag(Parser):
    def __init__(self, tag):
        self.tag = tag

    def __call__(self, tokens, pos):
        if pos < len(tokens) and tokens[pos][1] is self.tag:
            return Result(tokens[pos][0], pos + 1)

class Reserved(Parser):
    def __init__(self, value, tag):
        self.value = value
        self.tag = tag

    def __call__(self, tokens, pos):
        if (pos < len(tokens)
                and tokens[pos][0] == self.value
                and tokens[pos][1] is self.tag):
            return Result(tokens[pos][0], pos + 1)

class AddParser(Parser):
    def __init__(self, lparser, rparser):
        self.lparser = lparser
        self.rparser = rparser

    def __call__(self, tokens, pos):
        lresult = self.lparser(tokens, pos)
        if not lresult:
            return None

        rresult = self.rparser(tokens, lresult.pos)
        if rresult:
            value = (lresult.value, rresult.value)
            return Result(value, rresult.pos)

class MulParser(Parser):
    def __init__(self, parser, sep):
        self.parser = parser
        self.sep = sep

    def __call__(self, tokens, pos):
        result = self.parser(tokens, pos)

        def process_next(parsed):
            sep_func, pos = parsed
            return sep_func(result.value, pos)
        next_parser = self.sep + self.parser ^ process_next

        next_result = result
        while next_result:
            next_result = next_parser(tokens, result.pos)
            if next_result:
                result = next_result
        return result

class OrParser(Parser):
    def __init__(self, lparser, rparser):
        self.lparser = lparser
        self.rparser = rparser

    def __call__(self, tokens, pos):
        lresult = self.lparser(tokens, pos)
        if lresult:
            return lresult
        else:
            return self.rparser(tokens, pos)

class Option(Parser):
    def __init__(self, parser):
        self.parser = parser

    def __call__(self, tokens, pos):
        result = self.parser(tokens, pos)
        if result:
            return result
        else:
            return Result(None, pos)

class Repeat(Parser):
    def __init__(self, parser):
        self.parser = parser

    def __call__(self, tokens, pos):
        results = []
        while True:
            result = self.parser(tokens, pos)
            if not result:
                break
            results.append(result.value)
            pos = result.pos
        return Result(results, pos)

class Process(Parser):
    def __init__(self, parser, func):
        self.parser = parser
        self.func = func

    def __call__(self, tokens, pos):
        result = self.parser(tokens, pos)
        if result:
            result.value = self.func(result.value)
            return result

class Lazy(Parser):
    def __init__(self, parser_func):
        self.parser = None
        self.parser_func = parser_func

    def __call__(self, tokens, pos):
        if not self.parser:
            self.parser = self.parser_func()
        return self.parser(tokens, pos)

class Phrase(Parser):
    def __init__(self, parser):
        self.parser = parser

    def __call__(self, tokens, pos):
        result = self.parser(tokens, pos)
        if result and result.pos == len(tokens):
            return result
