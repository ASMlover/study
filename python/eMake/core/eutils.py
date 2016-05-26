#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

import os
import platform
import sys

if sys.version_info.major < 3:
    import codecs
    def eopen(fname, mode='rb', encoding=None):
        return codecs.open(fname, mode, encoding)
else:
    eopen = open

def singleton(cls):
    insts = {}
    def get_instance(*args, **kwargs):
        if cls not in insts:
            insts[cls] = cls(*args, **kwargs)
        return insts[cls]
    return get_instance

def get_platform():
    return platform.system().lower()

def get_sources_list(proj_path='./', exts=['cc', 'cpp'], fullpath=True):
    cur_sources = os.listdir(proj_path)
    all_sources = []

    for source_fname in cur_sources:
        source_fullpath = os.path.join(proj_path, source_fname).replace('\\', '/')
        if os.path.isdir(source_fullpath):
            next_sources = get_sources_list(source_fullpath, exts=exts, fullpath=fullpath)
            all_sources.extend(next_sources)
        else:
            if os.path.splitext(source_fname)[1][1:] in exts:
                if fullpath:
                    all_sources.append(source_fullpath)
                else:
                    all_sources.append(source_fname)
    return all_sources
