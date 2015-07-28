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

class BaseType(object):
    def __init__(self, val):
        self.val = val

    def __str__(self):
        pass

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.val == other.val

    def __lt__(self, other):
        return isinstance(other, self.__class__) and self.val < other.val

    def __ne__(self, other):
        return not self.__eq__(other)

    def __le__(self, other):
        return self.__lt__(other) or self.__eq__(other)

    def __gt__(self, other):
        return not self.__le__(other)

    def __ge__(self, other):
        return not self.__lt__(other)

    def __nonzero__(self):
        return True

class Number(BaseType):
    def __str__(self):
        return str(self.val)

class Boolean(BaseType):
    def __str__(self):
        return '#t' if self.val else '#f'

    def __nonzero__(self):
        return self.val

class Character(BaseType):
    def __str__(self):
        return '#\\%s' % str(self.val)

class String(BaseType):
    def __str__(self):
        return '"%s"' % str(self.val)

class Symbol(BaseType):
    def __str__(self):
        return self.val

class EmptyList(object):
    def __str__(self):
        return '()'

    def __eq__(self, other):
        return isinstance(other, self.__class__)

    def __nonzero__(self):
        return True

EmptyList = EmptyList()

class Pair(object):
    def __init__(self, head, tail):
        self.head = head
        self.tail = tail

    def __str__(self):
        return '(%s)' % self._write_pair(self)

    @staticmethod
    def _write_pair(pair):
        head, tail = pair.head, pair.tail

        output = str(head)
        if isinstance(tail, pair):
            output += ' %s' % Pair._write_pair(tail)
            return output

        if tail is EmptyList:
            return output

        output += ' . %s' % str(tail)
        return output
