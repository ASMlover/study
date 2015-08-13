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

from atom import TRUE, FALSE, Symbol
from seq import Sequence
from fun import Lambda

class Lisp(object):
    SPECIAL = '()'

    def dummy(self, env, args):
        self.println(env, args)

    def println(self, env, args):
        for arg in args:
            result = arg.eval(env)
            self.stdout.write('%s' % str(result))
        self.stdout.write('\n')

        return TRUE

    def cond(self, env, args):
        for arg in args:
            result = arg.car().eval(env)

            if result == TRUE:
                return arg.data[1].eval(env)

        return FALSE

    def eq(self, env, args):
        if len(args) > 2:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got {1}'.format(
                    2, len(args)))

        if args[0].eval(env) == args[1].eval(env):
            return TRUE
        return FALSE

    def quote(self, env, args):
        if len(args) > 1:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got {1}'.format(
                    1, len(args)))
        return args[0]

    def car(self, env, args):
        if len(args) > 1:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got {1}'.format(
                    1, len(args)))

        cell = args[0].eval(env)
        if not isinstance(cell, Sequence):
            raise ValueError(
                    'Function car not valid on non-sequence type: %s' % 
                    cell.data)

        return cell.car()

    def cdr(self, env, args):
        if len(args) > 1:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got{1}'.format(
                    1, len(args)))

        cell = args[0].eval(env)
        if not isinstance(cell, Sequence):
            raise ValueError(
                    'Function cdr not valid on non-sequence type: %s' % 
                    cell.data)

        return cell.cdr()

    def cons(self, env, args):
        if len(args) > 2:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got {1}'.format(
                    2, len(args)))

        first = args[0].eval(env)
        second = args[1].eval(env)

        return second.cons(first)

    def atom(self, env, args):
        if len(args) > 1:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got {1}'.format(
                    1, len(args)))
        first = args[0].eval(env)

        if first == FALSE:
            return TRUE
        elif isinstance(first, Symbol):
            return TRUE

        return FALSE

    def label(self, env, args):
        if len(args) != 2:
            raise ValueError(
                'Wrong number of arguments, expected {0}, got {1}'.format(
                    2, len(args)))

        env.set(args[0].data, args[1].eval(env))
        return env.get(args[0].data)
