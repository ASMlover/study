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

import collections
import dis
import inspect
import sys
import types

class Stack(list):
    def push(self, value):
        super(Stack, self).append(value)

    def popn(self, n=1):
        if n <= 1:
            return self.pop()

        ret = self[-n:]
        self[-n:] = []
        return ret

    def top(self):
        return self[-1]

    def size(self):
        return len(self)

class VMError(Exception):
    pass

class Frame(object):
    def __init__(self, code_obj, global_names, local_names, prev_frame):
        self.code_obj = code_obj
        self.global_names = global_names
        self.local_names = local_names
        self.prev_frame = prev_frame
        self.code_stack = Stack()
        self.last_instruction = 0
        self.block_stack = Stack()
        if prev_frame:
            self.builtin_names = prev_frame.builtin_names
        else:
            self.builtin_names = local_names['__builtins__']
            if hasattr(self.builtin_names, '__dict__'):
                self.builtin_names = self.builtin_names.__dict__

    def _code_push(self, code):
        self.code_stack.push(code)

    def _code_pushn(self, *codes):
        self.code_stack.extend(codes)

    def _code_pop(self):
        return self.code_stack.pop()

    def _code_popn(self, n):
        return self.code_stack.popn(n)

    def _code_top(self):
        return self.code_stack.top()

    def _code_size(self):
        return self.code_stack.size()

    def _block_push(self, block):
        self.block_stack.push(block)

    def _block_pop(self):
        return self.block_stack.pop()

    def _block_top(self):
        return self.block_stack.top()

    def _block_size(self):
        return self.block_stack.size()

class Function(object):
    __slots__ = [
        'func_code',
        'func_name',
        'func_defaults',
        'func_globals',
        'func_locals',
        'func_dict',
        'func_closure',
        '__name__',
        '__dict__',
        '__doc__',
        '_vm',
        '_func',
    ]
    def __init__(self, name, code, globs, defaults, closure, vm):
        self._vm = vm
        self.func_code = code
        self.func_name = self.__name__ = name or code.co_name
        self.func_defaults = tuple(defaults)
        self.func_globals = globs
        self.func_locals = self._vm.frame.f_locals
        self.func_closure = closure
        self.__dict__ = {}
        self.__doc__ = code.co_consts and code.co_consts[0] or None
        kw = {'argdefs': self.func_defaults}
        if closure:
            kw['closure'] = tuple(make_cell(0) for _ in closure)
        self._func = types.FunctionType(code, globs, **kw)

    def __call__(self, *args, **kwargs):
        callargs = inspect.getcallargs(self._func, *args, **kwargs)
        frame = self._vm.make_frame(
                self.func_code, callargs, self.func_globals, {})
        return self._vm.run_frame(frame)

def make_cell(value):
    fun = (lambda x: lambda: x)(value)
    return fun.__closure__[0]

Block = collections.namedtuple('Block', 'type, handler, stack_height')

class VM(object):
    def __init__(self):
        self.frame_stack = Stack()
        self.frame = None
        self.return_value = None
        self.last_exception = None

    def _push(self, value):
        self.frame_stack.push(value)

    def _pop(self):
        return self.frame_stack.pop()

    def _top(self):
        return self.frame_stack.top()

    def _frame_code_top(self):
        return self.frame._code_top()

    def _frame_code_pop(self):
        return self.frame._code_pop()

    def _frame_code_pushn(self, *vals):
        self.frame._code_pushn(*vals)

    def _frame_code_popn(self, n):
        if n:
            return self.frame._code_popn(n)
        else:
            return []

    def make_frame(self, code, callargs={}, global_names=None, local_names=None):
        if global_names is not None and local_names is not None:
            local_names = global_names
        elif self.frame_stack:
            global_names = self.frame.global_names
            local_names = {}
        else:
            global_names = local_names = {
                '__builtins__': __builtins__,
                '__name__': '__main__',
                '__doc__': None,
                '__package__': None,
            }
        local_names.update(callargs)
        frame = Frame(code, global_names, local_names, self.frame)
        return frame

    def push_frame(self, frame):
        self._push(frame)
        self.frame = frame

    def pop_frame(self):
        self._pop()
        if self.frame_stack:
            self.frame = self._top()
        else:
            self.frame = None

    def run_frame(self, frame):
        self.push_frame(frame)
        while True:
            byte_name, arguments = self.parse_byte_and_args()
            why = self.dispatch(byte_name, arguments)
            while why and frame.block_stack:
                why = self.manage_block_stack(why)

            if why:
                break
        self.pop_frame()

        if why == 'exception':
            exc, val, tb = self.last_exception
            e = exc(val)
            e.__traceback__ = tb
            raise e
        return self.return_value

    def parse_byte_and_args(self):
        f = self.frame
        op_offset = f.last_instruction
        byte_code = f.code_obj.co_code[op_offset]
        f.last_instruction += 1
        byte_name = dis.opname[byte_code]
        if byte_code >= dis.HAVE_ARGUMENT:
            arg = f.code_obj.co_code[f.last_instruction:f.last_instruction+2]
            f.last_instruction += 2
            arg_val = arg[0] + (arg[1] * 256)
            if byte_code in dis.hasconst:
                arg = f.code_obj.co_consts[arg_val]
            elif byte_code in dis.hasname:
                arg = f.code_obj.co_names[arg_val]
            elif byte_code in dis.haslocal:
                arg = f.code_obj.co_varnames[arg_val]
            elif byte_code in dis.hasjrel:
                arg = f.last_instruction + arg_val
            else:
                arg = arg_val
            argument = [arg]
        else:
            argument = []

        return byte_name, argument

    def dispatch(self, byte_name, argument):
        why = None
        try:
            bytecode_fn = getattr(self, '_ir_%s' % byte_name, None)
            if bytecode_fn is None:
                if byte_name.startswith('unary_'):
                    self.unary_operation(byte_name[6:])
                elif byte_name.startswith('binary_'):
                    self.binary_operation(byte_name[7:])
                else:
                    raise VMError("unsupported bytecode type: %s" % byte_name)
            else:
                why = bytecode_fn(*argument)
        except:
            self.last_exception = sys.exc_info()[:2] + (None,)
            why = 'exception'
        return why

    def push_block(self, b_type, handler=None):
        stack_height = self.frame._block_size()
        self.frame._block_push(Block(b_type, handler, stack_height))

    def pop_block(self):
        return self.frame._block_pop()

    def unwind_block(self, block):
        if block.type == 'except-handler':
            offset = 3
        else:
            offset = 0

        while self.frame._code_size() > block.level + offset:
            traceback, value, exctype = self._frame_code_popn(3)
            self.last_exception = exctype, value, traceback

    def manage_block_stack(self, why):
        frame = self.frame
        block = frame._block_top()
        if block.type == 'loop' and why == 'continue':
            self.jump(self.return_value)
            why = None
            return why
        self.pop_block()
        self.unwind_block(block)

def main():
    pass

if __name__ == '__main__':
    main()
