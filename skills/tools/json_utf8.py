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

"""通过json.loads转换后的dict中的key和value都是unicode,
一个简单的函数将其转换为utf8格式
"""

import json

def json_utf8loads(json_str):
    def byteify(attr):
        if isinstance(attr, dict):
            return {byteify(key): byteify(val) for key, val in attr.iteritems()}
        elif isinstance(attr, list):
            return [byteify(val) for val in attr]
        elif isinstance(attr, unicode):
            return attr.encode('utf-8')
        else:
            return attr

    return byteify(json.loads(json_str))

def main():
    test_dict = dict(
        id = 1000001,
        name = '中文名字',
        age = 20,
        level = 44,
        package = {1001: {'precious':True, 'count':1}, 1002:{'count':999}},
        equips = [{4001: {'strength': 100, 'magic': 33}}, {4002: {'hp': 99}}],
    )
    json_str = json.dumps(test_dict)
    print 'normal json loads: %s' % json.loads(json_str)
    print 'utf-8 json loads: %s' % json_utf8loads(json_str)

if __name__ == '__main__':
    main()
