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

import dis
import inspect
import linecache
import logging
import operator
import sys
import six
from six.moves import reprlib

from vm_obj import Block, Frame, Function, Generator, Method

_logger = logging.getLogger(__name__)
byteint = lambda b: b

repr_obj = reprlib.Repr()
repr_obj.maxother = 120
repper = repr_obj.repr

class VMError(Exception):
    pass

class VM(object):
    def __init__(self):
        self.frames = []
        self.frame = None
        self.return_value = None
        self.last_exception = None

    def top(self):
        return self.frame.stack[-1]

    def pop(self, i=0):
        return self.frame.stack.pop(-1 - i)

    def popn(self, n):
        if n:
            ret = self.frame.stack[-n:]
            self.frame.stack[-n:] = []
            return ret
        else:
            return []

    def push(self, *vals):
        self.frame.stack.extend(vals)

    def peek(self, n):
        return self.frame.stack[-n]

    def jump(self, jump):
        self.frame.f_lasti = jump

    def push_block(self, type, handler=None, level=None):
        if level is None:
            level = len(self.frame.stack)
        self.frame.block_stack.append(Block(type, handler, level))

    def pop_block(self):
        return self.frame.block_stack.pop()

    def make_frame(self, code, callargs={}, f_globals=None, f_locals=None):
        _logger.info('make_frame: code=%r, callargs=%r', code, repr(callargs))
        if f_globals is not None:
            f_globals = f_locals
            if f_locals is None:
                f_locals = f_globals
        elif self.frames:
            f_globals = self.frame.f_globals
            f_locals = {}
        else:
            f_globals = f_locals = {
                '__builtins__': __builtins__,
                '__name__': '__main__',
                '__doc__': None,
                '__package__': None,
            }
        f_locals.update(callargs)
        frame = Frame(code, f_globals, f_locals, self.frame)
        return frame

    def push_frame(self, frame):
        self.frames.append(frame)
        self.frame = frame

    def pop_frame(self):
        self.frame.pop()
        if self.frames:
            self.frame = self.frames[-1]
        else:
            self.frame = None

    def print_frame(self):
        for f in self.frames:
            fname = f.f_code.co_filename
            lineno = f.line_number()
            print ('\tFile "%s", line %d, in %s' % (
                fname, lineno, f.f_code.co_name))
            linecache.checkcache(fname)
            line = linecache.getline(fname, lineno, f.f_globals)
            if line:
                print('\t%s' % line.strip())

    def resume_frame(self, frame):
        frame.f_back = self.frame
        value = self.run_frame(frame)
        frame.f_back = None
        return value

    def run_code(self, code, f_globals=None, f_locals=None):
        frame = self.make_frame(code, f_globals=f_globals, f_locals=f_locals)
        value = self.run_frame(frame)
        if self.frames:
            raise VMError('Frames left over')
        if self.frame and self.frame.stack:
            raise VMError('Data left on stack! %r' % self.frame.stack)

        return value

    def unwind_block(self, block):
        if block.type == 'except-handler':
            offset = 3
        else:
            offset = 0

        while len(self.frame.stack) > block.level + offset:
            self.pop()

        if block.type == 'except-handler':
            tb, value, exctype = self.popn(3)
            self.last_exception = exctype, value, tb

    def parse_ir_and_args(self):
        f = self.frame
        opoffset = f.f_lasti
        byte_code = byteint(f.f_code.co_code[opoffset])
        f.f_lasti += 1
        byte_name = dis.opname[byte_code]
        arg = None
        arguments = []
        if byte_code >= dis.HAVE_ARGUMENT:
            arg = f.f_code.co_code[f.f_lasti:f.f_lasti+2]
            f.f_lasti += 2
            int_arg = byteint(arg[0]) + (byteint(arg[1]) << 8)
            if byte_code in dis.hasconst:
                arg = f.f_code.co_consts[int_arg]
            elif byte_code in dis.hasfree:
                if int_arg < len(f.f_code.co_callvars):
                    arg = f.f_code.co_callvars[int_arg]
                else:
                    var_idx = int_arg - len(f.f_code.co_cellvars)
                    arg = f.f_code.co_freevars[var_idx]
            elif byte_code in dis.hasname:
                arg = f.f_code.co_names[int_arg]
            elif byte_code in dis.hasjrel:
                arg = f.f_lasti + int_arg
            elif byte_code in dis.hasjabs:
                arg = int_arg
            elif byte_code in dis.haslocal:
                arg = f.f_code.co_varnames[int_arg]
            else:
                arg = int_arg
            arguments = [arg]

        return byte_name, arguments, opoffset

    def log(self, byte_name, arguments, opoffset):
        op = '%d: %s' % (opoffset, byte_name)
        if arguments:
            op += ' %r' % arguments[0]
        indent = '\t' * (len(self.frames) - 1)
        stack_rep = repper(self.frame.stack)
        block_stack_rep = repper(self.frame.block_stack)

        _logger.info('\t%sdata: %s', indent, stack_rep)
        _logger.info('\t%sblks: %s', indent, block_stack_rep)
        _logger.info('%s%s', indent, op)

    def dispatch(self, byte_name, arguments):
        pass
