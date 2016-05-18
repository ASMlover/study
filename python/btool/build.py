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

import argparse
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

def get_sources_list(root='./', fullpath=True):
    cur_sources = os.listdir(root)
    all_sources = []

    for source in cur_sources:
        source_fname = os.path.join(root, source)
        if os.path.isdir(source_fname):
            next_sources = get_sources_list(source_fname)
            all_sources.extend(next_sources)
        else:
            if os.path.splitext(source)[1][1:] in conf['extensions']:
                if fullpath:
                    all_sources.append(source_fname)
                else:
                    all_sources.append(source)

    return all_sources

def gen_options_string(key, options, functor=None, shave_last=False):
    OPTS_MAP = {
        'cflags': ' -%s',
        'preprocessor': ' -D%s',
        'inc_dir': ' -I"%s"',
        'ldflags': ' -%s',
        'lib_dir': ' -LIBPATH:"%s"',
        'dep_libs': ' %s',
        'srcs': '%s ',
        'objs': '%s.obj ',
    }

    if functor:
        options = ''.join((OPTS_MAP[key] % functor(opt) for opt in options))
    else:
        options = ''.join((OPTS_MAP[key] % opt for opt in options))
    if shave_last:
        options = options[:-1]

    return options

def gen_makefile_windows(target):
    cflags = gen_options_string('cflags', conf.get('compile_options', []))
    preprocessor = gen_options_string('preprocessor', conf.get('precompile_options', []))
    inc_dir = gen_options_string('inc_dir', conf.get('inc_dir', []))
    ldflags = gen_options_string('ldflags', conf.get('ldflags', []))
    lib_dir = gen_options_string('lib_dir', conf.get('lib_dir', []))
    dep_libs = gen_options_string('dep_libs', conf.get('dep_libraries', []))

    all_sources = get_sources_list(fullpath=False)
    srcs = gen_options_string('srcs', all_sources, shave_last=True)
    objs = gen_options_string('objs', all_sources, functor=lambda x: os.path.splitext(x)[0], shave_last=True)

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
    fun = GEN_FUNCTOR.get(pf)
    fun and fun(target)

def clean_windows():
    if os.path.exists('Makefile'):
        subprocess.check_call('nmake clean', shell=True)
        subprocess.check_call('del Makefile', shell=True)

def clean_linux():
    pass

def clean_darwin():
    pass

def build():
    pf = platform.system()
    gen_makefile(pf)
    subprocess.check_call('nmake', shell=True)

def rebuild():
    clean()
    build()

def clean():
    GEN_FUNCTOR = {
        'Windows': clean_windows,
        'Linux': clean_linux,
        'Darwin': clean_darwin
    }
    fun = GEN_FUNCTOR.get(platform.system())
    fun and fun()

def get_build_arguments():
    parser = argparse.ArgumentParser(description='C/C++ building tool')
    parser.add_argument('option', help='[build|rebuild|clean] the project')
    args = parser.parse_args()
    return args.option

def main():
    option = get_build_arguments()

    fun = getattr(sys.modules['__main__'], option, None)
    fun and fun()

if __name__ == '__main__':
    main()
