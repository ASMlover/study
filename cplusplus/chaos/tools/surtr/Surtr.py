#!/usr/bin/env python
# -*- coding: UTF-8 -*-
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
from __future__ import print_function

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys

if sys.version_info.major < 3:
    import codecs
    def do_open(fname, mode='rb', encoding=None):
        return codecs.open(fname, mode, encoding)
else:
    do_open = open

def get_platform():
    return platform.system().lower()

def safe_mkdir(dirpath):
    if not os.path.exists(dirpath):
        os.mkdir(dirpath)

def safe_rmdir(dirpath):
    if os.path.exists(dirpath):
        shutil.rmtree(dirpath)

def safe_rm(path):
    if os.path.exists(path):
        if os.path.isdir(path):
            safe_rmdir(path)
        else:
            os.remove(path)

def get_options():
    parser = argparse.ArgumentParser(description='Surtr C/C++ building tool')
    parser.add_argument('option', nargs='?', help='config|build|rebuild|clean|remove the project')
    args = parser.parse_args()
    return args.option

def get_conf(tool_dir='./', platform='linux'):
    ptname = 'posix'
    if platform == 'windows':
        ptname = 'windows'

    surtr_dir = '{tool_dir}/surtr/templates'.format(tool_dir=tool_dir)

    # get building Makefile template
    fname = '{surtr_dir}/build.{pt}.mk'.format(surtr_dir=surtr_dir, pt=ptname)
    with do_open(fname, 'r', encoding='utf-8') as fp:
        build_mk = fp.read()

    # get object generation template
    fname = '{surtr_dir}/obj.{pt}.mk'.format(surtr_dir=surtr_dir, pt=ptname)
    with do_open(fname, 'r', encoding='utf-8') as fp:
        obj_mk = fp.read()

    # get default build configure
    try:
        with do_open('surtr.conf', 'r', encoding='utf-8') as fp:
            build_conf = json.load(fp)
    except Exception:
        fname = '{surtr_dir}/default.{pt}.conf'.format(surtr_dir=surtr_dir, pt=ptname)
        with do_open(fname, 'r', encoding='utf-8') as fp:
            build_conf = json.load(fp)
    return build_mk, obj_mk, build_conf

def get_sources_for_dir(dirpath, recursive=True):
    cur_sources = os.listdir(dirpath)
    all_sources = []
    for source_name in cur_sources:
        source_fullpath = os.path.join(dirpath, source_name).replace('\\', '/')
        if os.path.isdir(source_fullpath) and recursive:
            all_sources.extend(get_sources_for_dir(source_fullpath))
        else:
            if os.path.splitext(source_name)[1][1:] in ('cpp', 'cc'):
                all_sources.append(source_fullpath)
    return all_sources

def get_all_sources(dirs=(('./', True),)):
    all_sources = []
    for path, recursive in dirs:
        all_sources.extend(get_sources_for_dir(path, recursive))
    return all_sources

def gen_outobj(source_fname, posix=True):
    s = source_fname.strip('./').strip('../').replace('/', '.')
    objname = os.path.splitext(s)[0]
    if posix:
        outobj_format = '$(OUTDIR)/$(OUTOBJ)/{objname}.o'
    else:
        outobj_format = '$(OUTDIR)\$(OUTOBJ)\{objname}.obj'
    return outobj_format.format(objname=objname)

def gen_makefile(platform='linux', target='a.out', outdir='build', sources=[]):
    module = sys.modules['__main__']

    objs_list = []
    makeobjs_list = []
    for s in sources:
        objstr = gen_outobj(s, platform != 'windows')
        objs_list.append(objstr)
        makeobjs_list.append()

def main():
    # Just for testing
    bmk, omk, bconf = get_conf('../')
    print (bmk)
    print ('********************************************************************')
    print (omk)
    print ('********************************************************************')
    print (bconf)

if __name__ == '__main__':
    main()
