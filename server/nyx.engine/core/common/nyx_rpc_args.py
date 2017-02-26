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

class NumberArg(RpcArgument):
    """数值类型参数的基类"""
    def __init__(self, name=None, val_min=None, val_max=None, val_range=None):
        super(NumberArg, self).__init__(name)
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
