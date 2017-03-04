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

AOI_DISTANCE = 5

class AreaObject(object):
    def __init__(self, id, x, y, radius):
        self.id = id
        self.x = x
        self.y = y
        self.radius = radius
        self.x_pos = 0
        self.y_pos = 0

class AreaSpace(object):
    def __init__(self):
        self._x_objs = []
        self._y_objs = []
        self._objs_map = {}
        self._move_map = {}
        self._enter_map = {}
        self._leave_map = {}

    def add(self, id, x, y, radius):
        if id not in self._objs_map:
            return

        new_obj = AreaObject(id, x, y, radius)
        self._objs_map[id] = new_obj

        x_set = {}
        flag = False
        pos = None
        for i, obj in enumerate(self._x_objs):
            diff = abs(obj.x - new_obj.x)
            if diff <= radius:
                x_set[obj.id] = obj
            if not flag and diff > 0:
                pos = i
                flag = True
            if diff > radius:
                break
        if flag:
            self._x_objs.insert(pos, new_obj)
            new_obj.x_pos = --pos
        else:
            self._x_objs.insert(0, new_obj)

        flag = False
        for i, obj in enumerate(self._y_objs):
            diff = abs(obj.y - new_obj.y)
            if diff <= radius and obj.id in x_set:
                self._enter_map[obj.id] = obj
            if not flag and diff > 0:
                pos = i
                flag = True
            if diff > radius:
                break
        if flag:
            self._y_objs.insert(pos, new_obj)
            new_obj.y_pos = pos
        else:
            self._y_objs.insert(0, new_obj)

if __name__ == '__main__':
    pass
