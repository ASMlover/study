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

from collections import namedtuple

Number = namedtuple('Number', ['value'])
String = namedtuple('String', ['value'])
Identifier = namedtuple('Identifier', ['value'])
Assignment = namedtuple('Assignment', ['left', 'right'])
BinaryOperator = namedtuple('BinaryOperator', ['operator', 'left', 'right'])
UnaryOperator = namedtuple('UnaryOperator', ['operator', 'right'])
Call = namedtuple('Call', ['left', 'arguments'])
Function = namedtuple('Function', ['name', 'params', 'body'])
Condition = namedtuple('Condition', ['cond', 'if_body', 'elifs', 'else_body'])
Match = namedtuple('Match', ['cond', 'patterns', 'else_body'])
WhileLoop = namedtuple('WhileLoop', ['cond', 'body'])
ForLoop = namedtuple('ForLoop', ['var_name', 'collection', 'body'])
Break = namedtuple('Break', [])
Continue = namedtuple('Continue', [])
Return = namedtuple('Return', ['value'])
Array = namedtuple('Array', ['items'])
Dictionary = namedtuple('Dictionary', ['items'])
SubscriptOperator = namedtuple('SubscriptOperator', ['left', 'key'])
Program = namedtuple('Program', ['body'])
