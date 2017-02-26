#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2017 ASMlover. All rights reserved.
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

from common.nyx_id import IdCreator

class ConvertError(ValueError):
    """RPC调用的参数转换错误"""
    pass

class RpcArgument(object):
    def __init__(self, name):
        super(RpcArgument, self).__init__()
        self.name = name

    def get_name(self):
        """获取rpc参数名"""
        return self.name

    def convert(self, data):
        raise ConvertError('Not implemented!')

    def get_type(self):
        raise ConvertError('Not implemented!')

    def get_name_type(self):
        return '<%s(%s)>' % (self.get_name(), self.get_type())

    def default_val(self):
        raise StandardError('Not implemented!')

    def to_string(self, value):
        return str(value)

    def __str__(self):
        return self.get_name_type()

    def convert_error(self, data):
        return ConvertError('Cannot convert Argument(%s) to Type(%s) with Data(%s)' % (self.name, self.get_type(), data))

class NoLimit(object):
    def is_valid(self, value):
        return True

    def __str__(self):
        return ''

class NumeralLimit(object):
    def __init__(self, val_min=None, val_max=None, val_range=None):
        super(NumeralLimit, self).__init__()
        self._min = val_min
        self._max = val_max
        self._range = val_range

    def is_valid(self, value):
        if self._min is not None and value < self._min:
            return False
        if self._max is not None and value > self._max:
            return False
        if self._range is not None and value not in self._range:
            return False
        return True

    def __str__(self):
        str_repr = ''
        if self._min is not None or self._max is not None:
            str_repr = '['
            if self._min is not None:
                str_repr += str(self._min)
            str_repr += '-'
            if self._max is not None:
                str_repr += str(self._max)
            str_repr += ']'
        elif self._range is not None:
            str_repr = str(self._range).replace(' ', '')
        return str_repr

class Number(RpcArgument):
    """数值类型参数的基类"""
    def __init__(self, name=None, val_min=None, val_max=None, val_range=None):
        super(Number, self).__init__(name)
        if val_min is not None or val_max is not None or val_range is not None:
            self._limited = NumeralLimit(val_min=val_min, val_max=val_max, val_range=val_range)

    def convert(self, data):
        try:
            value = self.convert(data)
        except:
            raise ConvertError('Cannot Convert Data(%r) to Type(%s)' % (data, self.get_type()))

        if hasattr(self, '_limited') and not self._limited.is_valid(data):
            raise ConvertError('Data(%r) exceeds limit of Type(%s)'% (data, self.get_type()))

        return value

class Bool(RpcArgument):
    def convert(self, value):
        try:
            return bool(value)
        except:
            raise self.convert_error(value)

    def get_type(self):
        return '<Bool>'

    def default_val(self):
        return False

class Int(Number):
    def convert(self, value):
        assert isinstance(value, int), self.convert_error(value)
        return value

    def get_type(self):
        if hasattr(self, '_limited'):
            return '<Int>%s' % str(self._limited)
        else:
            return '<Int>'

    def default_val(self):
        return 0

class Float(Number):
    def convert(self, value):
        assert isinstance(value, float) or isinstance(value, int), self.convert_error(value)
        return float(value)

    def get_type(self):
        if hasattr(self, '_limited'):
            return '<Float>%s' % str(self._limited)
        else:
            return '<Float>'

    def default_val(self):
        return 0.0

class Str(RpcArgument):
    def convert(self, value):
        assert isinstance(value, str) or isinstance(value, unicode), self.convert_error(value)
        return str(value)

    def get_type(self):
        return '<Str>'

    def default_val(self):
        return ''

class Binary(RpcArgument):
    def convert(self, data):
        assert isinstance(data, str), self.convert_error(data)
        return data

    def get_type(self):
        return '<Binary>'

    def default_val(self):
        return ''

class Tuple(RpcArgument):
    def convert(self, data):
        assert isinstance(data, tuple) or isinstance(data, list), self.convert_error(data)
        return tuple(data)

    def get_type(self):
        return '<Tuple>'

    def default_val(self):
        return ()

def List(RpcArgument):
    def convert(self, data):
        assert isinstance(data, list), self.convert_error(data)
        return data

    def get_type(self):
        return '<List>'

    def default_val(self):
        return []

def Dict(RpcArgument):
    def convert(self, data):
        assert isinstance(data, dict), self.convert_error(data)
        return data

    def get_type(self):
        return '<Dict>'

    def default_val(self):
        return {}

class Uuid(RpcArgument):
    def convert(self, data):
        if data is None:
            return ''
        assert IdCreator.is_valid(data), self.convert_error(data)
        return data

    def get_type(self):
        return '<Uuid>'

    def default_val(self):
        return ''

class Exposed(Uuid):
    def get_type(self):
        return '<Exposed>'

class Avatar(RpcArgument):
    """客户端调用服务端"""
    def __init__(self, name='Avatar'):
        super(Avatar, self).__init__(name)

class Mailbox(RpcArgument):
    """服务端调用服务端"""
    def __init__(self, name='Mailbox'):
        super(Mailbox, self).__init__(name)

NYXRPC_CONLY = 0 # client -> server
NYXRPC_CANY = 1 # any client -> server
NYXRPC_SONLY = 2 # server -> server
NYXRPC_BOTH = 3 # client/server -> server
NYXRPC_CSTUB = 4 # server -> client

def rpc_call(func, parameters=None):
    args = []

    if parameters:
        if isinstance(parameters, list):
            parameters = {'__args': parameters}
        '__args' in parameters and parameters.update(dict(enumerate(parameters.pop('__args'))))
        for arg_type in func.arg_types:
            arg_name = arg_type.get_name()
            args.append(arg_type.convert(parameters[arg_name]))
    return func(*args)

def rpc_method(rpc_type, *arg_types):
    assert (not filter(lambda t: not hasattr(t, 'convert'), arg_types)), 'Bad argument type(s)'
    assert (rpc_type in (NYXRPC_CONLY, NYXRPC_CANY, NYXRPC_SONLY, NYXRPC_BOTH, NYXRPC_CSTUB)), \
            '%s: type must be one of (NYXRPC_CONLY, NYXRPC_CANY, NYXRPC_SONLY, NYXRPC_BOTH, NYXRPC_CSTUB)' % str(type)

    def _auto_name_argtypes(arg_types):
        n = 0
        for arg_type in arg_types:
            if arg_type.name is None:
                arg_type.name = n
                n += 1

    if rpc_type == NYXRPC_CANY:
        arg_types = (Exposed(),) + arg_types
    _auto_name_argtypes(arg_types)

    def _rpc_wrapper(func):
        func.rpc_type = rpc_type
        func.arg_types = arg_types
        return func
    return _rpc_wrapper

def is_rpc(func):
    """是否是rpc方法"""
    return hasattr(func, 'rpc_type')

def is_unexposed_to_client(func):
    """是否RPC不允许客户端调用(rpc_type需要是NYXRPC_CONLY, NYXRPC_CANY, NYXRPC_BOTH)"""
    return not is_rpc(func) or func.rpc_type not in (NYXRPC_CONLY, NYXRPC_CANY, NYXRPC_BOTH)

def is_unexposed_to_server(func):
    """是否RPC不允许服务端调用(rpc_type需要是NYXRPC_SONLY, NYXRPC_BOTH)"""
    return not is_rpc(func) or func.rpc_type not in (NYXRPC_SONLY, NYXRPC_BOTH)
