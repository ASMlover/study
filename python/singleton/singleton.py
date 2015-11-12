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

class Singleton(object):
    """
    The Singleton class decorator.

    @Singleton
    class MyClass(object):
        def __init__(self):
            pass

    use MyClass.instance() get the instance
    """

    def __init__(self, cls):
        self.__cls = cls
        self.__instance = None

    def initialize(self, *args, **kwargs):
        if not self.is_initialized():
            self.__instance = self.__cls(*args, **kwargs)

    def is_initialized(self):
        return self.__instance is not None

    def instance(self):
        if not self.is_initialized():
            self.initialize()
        return self.__instance

    def __call__(self, *args, **kwargs):
        raise TypeError('Singletons must be access by instance')

    def __instancecheck__(self, inst):
        return isinstance(inst, self.__cls)


from threading import Lock
class SafeSingleton(object):
    def __init__(self, cls):
        self.__cls = cls
        self.__instance = None
        self.__mutex = Lock()

    def initialize(self, *args, **kwargs):
        self.__mutex.acquire()
        try:
            if self.__instance is None:
                self.__instance = self.__cls(*args, **kwargs)
        finally:
            self.__mutex.release()

    def is_initialized(self):
        self.__mutex.acquire()
        try:
            return self.__instance is not None
        finally:
            self.__mutex.release()

    def instance(self):
        self.__mutex.acquire()
        try:
            if self.__instance is None:
                self.__instance = self.__cls()
            return self.__instance
        finally:
            self.__mutex.release()

    def __call__(self, *args, **kwargs):
        raise TypeError('Singletons must be access by instance')

    def __instancecheck__(self, inst):
        return isinstance(inst, self.__cls)
