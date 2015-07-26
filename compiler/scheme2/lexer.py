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

import error
import token

class Lex(object):
    def __init__(self, scan):
        self.scan = scan
        self.list = []
        self.paren_depth = 0

    def lex(self):
        scan = self.scan
        self.list = []
        while scan.next():
            pos = scan.get_pos()
            if scan.get() == '(':
                self.list.append(token.Token('bparen', scan.emit(), pos))
                self.paren_depth += 1
                self.lex_list()
            elif scan.get() == ')':
                self.list.append(token.Token('eparen', scan.emit(), pos))
                self.paren_depth -= 1
                if self.paren_depth > 0:
                    self.lex_list()
                elif self.paren_depth < 0:
                    self.paren_depth += 1
                    raise error.Error('too many end parens').error()
                else:
                    return self.list
            elif scan.get() == '#':
                while scan.get() != '\n':
                    scan.next()
                scan.emit()
            else:
                dump = scan.emit()
                if len(dump.strip()) > 0:
                    error.Error('unknown \'' + (dump if len(dump) <= 10 else dump[0:10-3] + '...') + '\'', pos=scan.get_pos()).error()
        return self.list

    def lex_list(self):
        scan = self.scan
        self.lex_space()
        while True:
            if not self.lex_id():
                if not self.lex_number():
                    if not self.lex_str():
                        break
            self.lex_space()
    
    def lex_space(self):
        scan = self.scan
        while scan.next():
            if not scan.get().isspace():
                scan.backup()
                break
        scan.emit()

    def lex_id(self):
        scan = self.scan
        pos = scan.get_pos()
        if not scan.next():
            return False
        if not scan.get().isalpha() and not self.is_symbol(scan.get()):
            scan.backup()
            return False
        while scan.next():
            if not scan.get().isalnum() and not self.is_symbol(scan.get()):
                scan.backup()
                break
        self.list.append(token.Token('id', scan.emit(), pos))
        return True

    def lex_number(self):
        scan = self.scan
        pos = scan.get_pos()
        while scan.next():
            if not scan.get().isdigit():
                scan.backup()
                break
        if scan.len() == 0:
            return False
        self.list.append(token.Token('number', scan.emit(), pos))
        return True

    def lex_str(self):
        scan = self.scan
        pos = scan.get_pos()
        scan.next()
        if scan.get() == '"':
            while scan.next():
                if scan.get() == '"':
                    break
        else:
            scan.backup()
            return False
        self.list.append(token.Token('str', scan.emit()[1:-1], pos))
        return True

    def is_symbol(self, c):
        return c in [
            ':',
            '!',
            '`',
            '~',
            '@',
            '#',
            '$',
            '%',
            '^',
            '&',
            '+', 
            '-',
            '*',
            '/',
            '=',
            '>',
            '<',
            '{',
            '}',
            '[',
            ']',
            '.',
        ]
