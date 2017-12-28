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

def merge(cls_array):
    if not cls_array:
        return []

    for mros in cls_array:
        for cls in mros:
            for cls_mros in cls_array:
                if cls in cls_mros[1:]:
                    break
            else:
                next_merge_mros = []
                for cls_mros in cls_array:
                    if cls in cls_mros:
                        cls_mros.remove(cls)
                        if cls_mros:
                            next_merge_mros.append(cls_mros)
                    else:
                        next_merge_mros.append(cls_mros)
                return [cls] + merge(next_merge_mros)

def c3mro(cls):
    if len(cls.__bases__) == 1:
        return [cls, cls.__base__]
    else:
        mro_parent = [c3mro(c) for c in cls.__bases__]
        mro_parent.append([c for c in cls.__bases__])
        return [cls] + merge(mro_parent)

if __name__ == '__main__':
    class A(object): pass
    class B(object): pass
    class C(object): pass
    class D(object): pass
    class E(object): pass
    class X(A, B, C): pass
    class Y(D, B, E): pass
    class Z(D, A): pass
    class U(X, Y, Z): pass
    print 'X.__mro__: ', [c.__name__ for c in c3mro(X)]
    print 'Y.__mro__: ', [c.__name__ for c in c3mro(Y)]
    print 'Z.__mro__: ', [c.__name__ for c in c3mro(Z)]
    print 'U.__mro__: ', [c.__name__ for c in c3mro(U)]
