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

class DirtyManager(object):
    _dirties = {}

    @staticmethod
    def add_dirty_state(id, is_dirty):
        DirtyManager._dirties[id] = is_dirty

    @staticmethod
    def del_dirty_state(id):
        DirtyManager._dirties.pop(id, None)

    @staticmethod
    def set_dirty_state(id, is_dirty):
        if id in DirtyManager._dirties:
            DirtyManager._dirties[id] = is_dirty

    @staticmethod
    def get_dirty_state(id):
        return DirtyManager._dirties.get(id, False)

class EntityProxy(object):
    """服务端Entity的代理"""
    def __init__(self, entity, mailbox):
        self.entity = entity
        self.mailbox = mailbox

    def __getattr__(self, name):
        def _caller(*args):
            if args:
                parameters = {'_': args}
            else:
                parameters = None
            self.entity.call_server_method(self.mailbox, name, parameters)
        return _caller
