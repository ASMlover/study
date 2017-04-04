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

import json
import os
import platform
import sys

class ExtendableType(type):
    def __new__(cls, name, bases, dict_info):
        if name == '__extend__':
            for cls in bases:
                for key, value in dict_info.iteritems():
                    if key == '__module__':
                        continue
                    setattr(cls, key, value)
            return None
        else:
            return super(ExtendableType, cls).__new__(cls, name, bases, dict_info)

class AttrProxy(object):
    def __init__(self, attr):
        super(AttrProxy, self).__setattr__('attr', attr)

    def __setattr__(self, name, value):
        attr = super(AttrProxy, self).__getattribute__('attr')
        attr[name] = value

    def __getattr__(self, name):
        attr = super(AttrProxy, self).__getattribute__('attr')
        value = attr.get(name)
        if not value:
            raise AttributeError('AttrProxy has no attribute(%s)' % name)
        return value

class JsonConfig(object):
    @staticmethod
    def load(fname):
        with open(fname) as fp:
            return json.load(fp)

    @staticmethod
    def save(fname, jsonconf):
        with open(fname, 'w') as fp:
            json.dump(jsonconf, fp)

def singleton(cls):
    _instances = {}

    def get_instance(*args, **kwargs):
        if cls not in _instances:
            _instances[cls] = cls(*args, **kwargs)
        return _instances[cls]
    return get_instance

def get_architectuew():
    return platform.architecture()[0]

def insert_syspath(newpath):
    """将newpath添加到Python的sys.path中

    Returns:
        1 - 表示添加成功
        0 - 表示newpath已经在Python的sys.path中
        -1 - 表示newpath目录不存在
    """
    if not os.path.exists(newpath):
        return -1

    new_abspath = os.path.abspath(newpath)
    if sys.platform == 'win32':
        new_abspath = new_abspath.lower()
        for path in sys.path:
            abspath = os.path.abspath(path).lower()
            if new_abspath in (abspath, abspath + os.sep):
                return 0
    sys.path.append(new_abspath)
    return 1
