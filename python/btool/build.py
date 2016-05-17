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

import json
import os
import platform
import subprocess
import sys

try:
    with open('conf.json', 'r', encoding='utf-8') as fp:
        conf = json.load(fp)
except Exception:
    with open('template.conf.json', 'r', encoding='utf-8') as fp:
        conf = json.load(fp)

def get_sources_list(root='./'):
    cur_sources = os.listdir(root)
    all_sources = []

    for source in cur_sources:
        source_fname = os.path.join(root, source)
        if os.path.isdir(source_fname):
            next_sources = get_sources_list(source_fname)
            all_sources.extend(next_sources)
        else:
            if os.path.splitext(source)[1][1:] in conf['extensions']:
                all_sources.append(source)

    return all_sources

def gen_makefile_windows(target):
    # TODO: now just functional, need refactor
    cflags_list = conf.get('compile_options', [])
    cflags = ''.join((' -%s' % flags for flags in cflags_list))
    preprocessor_list = conf.get('precompile_options', [])
    preprocessor = ''.join((' -D%s' % flags for flags in preprocessor_list))
    inc_dir_list = conf.get('inc_dir', [])
    inc_dir = ''.join((' -I"%s"' % inc for inc in inc_dir_list))

    ldflags_list = conf.get('link_options', [])
    ldflags = ''.join((' -%s' % flags for flags in ldflags_list))
    lib_dir_list = conf.get('lib_dir', [])
    lib_dir = ''.join((' -LIBPATH:"%s"' % path for path in lib_dir_list))
    dep_libs_list = conf.get('dep_libraries', [])
    dep_libs = ''.join((' %s' % lib for lib in dep_libs_list))

    all_sources = get_sources_list()
    srcs = ''.join(('%s ' % src for src in all_sources))[:-1]
    objs = ''.join(('%s.obj ' % os.path.splitext(src)[0] for src in all_sources))[:-1]

    with open('./templates/Windows/bin.mk', 'r', encoding='utf-8') as rfp:
        mk = rfp.read().format(
            bin=target,
            cflags=cflags,
            preprocessor=preprocessor,
            inc_dir=inc_dir,
            ldflags=ldflags,
            lib_dir=lib_dir,
            dep_libs=dep_libs,
            srcs=srcs,
            objs=objs
        )
        with open('Makefile', 'w', encoding='utf-8') as wfp:
            wfp.write(mk)


def gen_makefile_linux(pf):
    pass

def gen_makefile_darwin(pf):
    pass

def gen_makefile(pf):
    GEN_FUNCTOR = {
        'Windows': gen_makefile_windows,
        'Linux': gen_makefile_linux,
        'Darwin': gen_makefile_darwin
    }

    target = conf['bin']
    GEN_FUNCTOR[pf](target)

def build():
    pf = platform.system()
    gen_makefile(pf)
    subprocess.check_call('nmake', shell=True)

def clean():
    pf = platform.system()
    if pf == 'Windows':
        if os.path.exists('Makefile'):
            subprocess.check_call('nmake clean', shell=True)
            subprocess.check_call('del Makefile', shell=True)

def main():
    # TODO: need use argparse
    if len(sys.argv) > 1:
        opt = sys.argv[1]
        fun = getattr(sys.modules['__main__'], opt, None)
        fun and fun()

if __name__ == '__main__':
    main()
