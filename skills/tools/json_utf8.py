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

def jsonloads_u8(json_str):
    def byteify(attr):
        if isinstance(attr, dict):
            return {byteify(k): byteify(v) for k, v in attr.iteritems()}
        elif isinstance(attr, list):
            return [byteify(v) for v in attr]
        elif isinstance(attr, unicode):
            return attr.encode('utf-8')
        else:
            return attr
    return byteify(json.loads(json_str))

def main():
    avatar = dict(
        id = 1000001,
        name = 'json转换测试',
        level = 44,
        equips = {4001: {'atk': 11, 'speed': 5}, 4002: {'magic': 13}},
        packages = [{1001: {'precious': True, 'count': 1}}, {1002: {'count': 99}}],
    )

    json_avatar = json.dumps(avatar)
    print 'json.loads: %s' % json.loads(json_avatar)
    print 'jsonloads_u8: %s' % jsonloads_u8(json_avatar)

if __name__ == '__main__':
    main()
