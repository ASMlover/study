#!/usr/bin/env python
# -*- coding: UTF-8 -*-
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

def object_type():
    a = 34
    b = a
    print ('id(a) = %d, id(b) = %d' % (id(a), id(b)))

    if a is b:
        print ('a and b are the same type')
    print ('type(a) = %s, type(b) = %s' % (type(a), type(b)))

    a = 34
    if isinstance(a, int):
        print ('%d is integer type' % a)
    a = 34.5
    if isinstance(a, float):
        print ('%f is float type' % a)
    a = True
    if isinstance(a, bool):
        print ('%s is boolean type' % a)
    a = 'Hello, world'
    if isinstance(a, str):
        print ('\'%s\' is string type' % a)
    a = [1, 2, 3, 4, 5]
    if isinstance(a, list):
        print (a, 'is list type')
    a = (1, 2, 3)
    if isinstance(a, tuple):
        print (a, 'is tuple type')
    a = {'name': 'tim', 'age': 20, 'sex': 'male'}
    if isinstance(a, dict):
        print (a, 'is dict type')

if __name__ == '__main__':
    object_type()
