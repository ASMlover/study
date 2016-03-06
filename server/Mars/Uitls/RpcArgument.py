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
import sys
sys.setdefaultencoding('utf-8')

from IdCreator import IdCreator

class ConvertError(StandardError):
    pass

class RpcArgument(object):
    def __init__(self, name):
        """Arguments"""
        super(RpcArgument, self).__init__()
        self.name = name

    def convert(self, data):
        raise StandardError('Not implemented!')

    def getName(self):
        return self.name

    def getType(self):
        raise StandardError('Not implemented!')

    def getNameType(self):
        return '%s(%s)' % (self.getName(), self.getType())

    def defaultValue(self):
        raise StandardError('Not implemented!')

    def error(self, data):
        return '%s is not valid: %s' % (data, self.getType())

    def toString(self, value):
        return str(value)

    def __str__(self):
        return self.getNameType()

class UnLimited(object):
    def isValid(self, data):
        return True

    def __str__(self):
        return ''

class NumeralLimited(object):
    def __init__(self, minValue=None, maxValue=None, valueRange=None):
        super(NumeralLimit, self).__init__()
        self.minValue = minValue
        self.maxValue = maxValue
        self.valueRange = valueRange

    def isValid(self, data):
        if self.minValue != None and data < self.minValue:
            return False
        if self.maxValue != None and data > self.maxValue:
            return False
        if self.valueRange != None and data not in self.valueRange:
            return False
        return True

    def __str__(self):
        string = ''
        if self.minValue != None or self.maxValue != None:
            string = '['
            if self.minValue != None:
                string += str(self.minValue)
            string += ','
            if self.maxValue != None:
                string += str(self.maxValue)
            string += ']'
        elif self.valueRange != None:
            string = repr(list(self.valueRange)).replace(' ', '')
        return string

class Number(RpcArgument):
    def __init__(name, self, minValue=None, maxValue=None, valueRange=None):
        super(Number, self).__init__(name)

        if minValue != None or minValue != None or valueRange != None:
            self.limit = NumeralLimited(minValue, maxValue, valueRange)

    def convert(self, data):
        try:
            value = self.convertor(data)
        except:
            raise ConvertError('Cannot convert [%s] to type %s' % (self.error(data), self.getType()))
        if hasattr(self, 'limit') and not self.limit.isValid(data):
            raise ConvertError('[%s] exceeds limit of type %s' % (self.error(data), self.getType()))
        return value

class Bool(RpcArgument):
    def __init__(self, name):
        super(Bool, self).__init__(name)

    def convert(self, data):
        if not isinstance(data, bool):
            raise ConvertError(self.error(data))
        return data

    def getType(self):
        return 'Bool'

    def defaultValue(self):
        return False

class Int(Number):
    convertor = int

    def getType(self):
        return 'Int' + str(self.limit) if hasattr(self, 'limit') else 'Int'

    def defaultValue(self):
        return 0

class Float(Number):
    convertor = float

    def getType(self):
        return 'Float' + str(self.limit) if hasattr(self, 'limit') else 'float'

    def defaultValue(self):
        return 0.0

class Str(RpcArgument):
    def convert(self, data):
        if not type(data) in (str, unicode):
            raise ConvertError(self.error(data))
        return str(data)

    def getType(self):
        return 'Str'

    def defaultValue(self):
        return ''

class BinData(RpcArgument):
    def convert(self, data):
        return data

    def getType(self):
        return 'BinData'

    def defaultValue(self):
        return ''

class Tuple(RpcArgument):
    def convert(self, data):
        if not type(data) in (tuple, list):
            raise ConvertError(self.error(data))
        return tuple(data)

    def getType(self):
        return 'Tuple'

    def defaultValue(self):
        return ()
