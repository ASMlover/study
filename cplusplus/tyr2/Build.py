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
import os
import platform
import shutil
import subprocess
import sys

MKFORMAT = """
OUT	= {build_dir}/bin/{out}
RM	= rm
CC	= {cc}
CFLAGS	= -g -O2 -Wall -std=c++0x
LDFLAGS	= {ldflags}
OBJS	= {objs}

all: $(OUT)
rebuild: clean all
clean:
	$(RM) $(OUT) $(OBJS)

$(OUT): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
{build_objs}
"""

COMPILEFORMAT = """{build_obj}: {build_src}
	$(CC) -o {build_obj} -c $(CFLAGS) {build_src}
"""

if sys.version_info.major < 3:
    import codecs
    def do_open(fname, mode='rb', encoding=None):
        return codecs.open(fname, mode, encoding)
else:
    do_open = open

def get_platform():
    return platform.system().lower()

def gen_build_dir(build_dir='build'):
    def _mkdir_env(path):
        if not os.path.exists(path):
            os.mkdir(path)
            print ('********** create directory `{path}` success'.format(path=path))
        else:
            print ('>>>>>>>>>> `{path}` exists'.format(path=path))
    _mkdir_env(build_dir)
    _mkdir_env('{bdir}/bin'.format(bdir=build_dir))
    _mkdir_env('{bdir}/obj'.format(bdir=build_dir))

def get_all_source_list(path_list=['./']):
    def _get_source_list(path):
        cur_sources = os.listdir(path)
        all_sources = []

        for source_name in cur_sources:
            source_fullpath = os.path.join(path, source_name).replace('\\', '/')
            if os.path.isdir(source_fullpath):
                next_sources = _get_source_list(source_fullpath)
                all_sources.extend(next_sources)
            else:
                if os.path.splitext(source_name)[1][1:] in ('c', 'cpp', 'cc'):
                    all_sources.append(source_fullpath)
        return all_sources

    all_sources = []
    for path in path_list:
        all_sources.extend(_get_source_list(path))
    return all_sources

def _gen_obj(s, build_dir):
    s = s.split('/')[-1]
    return '{build_dir}/obj/{obj}.o '.format(build_dir=build_dir, obj=os.path.splitext(s)[0])

def _gen_build_obj(o, s):
    return COMPILEFORMAT.format(build_obj=o.strip(), build_src=s)

def _gen_linux_makefile():
    cc = 'g++'
    ldflags = '-lpthread'
    return dict(cc=cc, ldflags=ldflags)

def _gen_darwin_makefile():
    cc = 'clang++'
    ldflags = '-lc -lpthread'
    return dict(cc=cc, ldflags=ldflags)

def gen_build_script(pt='linux', target='a.out', build_dir='build', source_list=[]):
    method_name = '_gen_{pt}_makefile'.format(pt=pt)
    method = getattr(sys.modules['__main__'], method_name, None)
    if method:
        objs_list = []
        build_objs_list = []
        for s in source_list:
            objstr = _gen_obj(s, build_dir)
            objs_list.append(objstr)
            build_objs_list.append(_gen_build_obj(objstr, s))

        mk_dict = dict(
            build_dir = build_dir,
            out = target,
            objs = ''.join(objs_list)[:-1],
            build_objs = ''.join(build_objs_list)[:-1]
        )
        mk_dict.update(method())
        with do_open('Makefile', 'w', encoding='utf-8') as fp:
            fp.write(MKFORMAT.format(**mk_dict))

        return True
    else:
        print ('>>>>>>>>>> `{pt}` platform does not support'.format(pt=pt))

def get_arguments():
    parser = argparse.ArgumentParser(description='C/C++ building tool')
    parser.add_argument('option', help='build|rebuild|clean|remove the project')
    args = parser.parse_args()
    return args.option

def remove(build_dir):
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    if os.path.exists('Makefile'):
        os.remove('Makefile')

def main():
    option = get_arguments()
    build_dir = 'build'

    if option == 'remove':
        remove(build_dir)
        return

    gen_build_dir(build_dir=build_dir)
    sources_list = get_all_source_list(path_list=['./'])
    if not gen_build_script(pt=get_platform(), target='tyr', build_dir=build_dir, source_list=sources_list):
        remove(build_dir)
        return

    if option == 'build':
        subprocess.check_call('make', shell=True)
    elif option == 'rebuild':
        subprocess.check_call('make rebuild', shell=True)
    elif option == 'clean':
        subprocess.check_call('make clean', shell=True)
    else:
        remove(build_dir)

if __name__ == '__main__':
    main()
