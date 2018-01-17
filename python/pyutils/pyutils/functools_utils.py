#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2018 ASMlover. All rights reserved.
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

def partial(func, *args, **keywords):
    def _wrapper(*fargs, **fkeywords):
        new_kwargs = keywords.copy()
        new_kwargs.update(fkeywords)
        return func(*(args + fargs), **new_kwargs)
    _wrapper.func = func
    _wrapper.args = args
    _wrapper.keywords = keywords
    return _wrapper

_WRAPPER_ASSIGNMENTS = ('__module__', '__name__', '__doc__')
_WRAPPER_UPDATES = ('__dict__',)
def update_wrapper(wrapper,
        wrapped, assigned=_WRAPPER_ASSIGNMENTS, updated=_WRAPPER_UPDATES):
    for attr in assigned:
        setattr(wrapper, attr, getattr(wrapped, attr))
    for attr in updated:
        getattr(wrapper, attr).update(getattr(wrapped, attr, {}))
    return wrapper

def wraps(wrapped, assigned=_WRAPPER_ASSIGNMENTS, updated=_WRAPPER_UPDATES):
    return partial(update_wrapper,
            wrapped=wrapped, assigned=assigned, updated=updated)

if __name__ == '__main__':
    new_int = partial(int, base=2)

    print new_int('101011')
