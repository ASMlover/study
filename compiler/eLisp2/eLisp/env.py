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

class Environment(object):
    """
    The binding are stored in a simple dict and the stack
    disciplin is emulated through the `parent` link.
    """
    def __init__(self, parent=None, binds=None):
        self.parent = parent
        self.binds = binds if binds else {}

        if parent:
            self.level = self.parent.level + 1
        else:
            self.level = 0

    def get(self, key):
        """
        Getting a binding potentially requires the traversal
        of the parent link.
        """
        if key in self.binds:
            return self.binds[key]
        elif self.parent:
            return self.parent.get(key)
        else:
            raise ValueError('Invalid symbol ' + key)

    def set(self, key, value):
        """
        Setting a binding is symmetric to getting.
        """
        if key in self.binds:
            self.binds[key] = value
        elif self.parent:
            self.parent.set(key, value)
        else:
            self.binds[key] = value

    def defined(self, key):
        return key in self.binds

    def push(self, binds=None):
        """
        Push a new binding by creating a new Environment.
        """
        return Environment(self, binds)

    def pop(self):
        return self.parent

    def __repr__(self):
        result = '\nEnvironment %s:\n' % self.level
        keys = [i for i in self.binds.keys() if not i[:2] == '__']

        for key in keys:
            result = '%s %5s: %s\n' % (result, key, self.binds[key])
        return result
