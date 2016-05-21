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

if sys.version_info.major < 3:
    raise RuntimeError('Python3 is required')

def get_conf():
    try:
        with open('conf.json', 'r', encoding='utf-8') as fp:
            conf = json.load(fp)
    except Exception:
        if platform.system() == 'Windows':
            conf_file = 'template.win.conf.json'
        else:
            conf_file = 'template.posix.conf.json'
        with open(conf_file, 'r', encoding='utf-8') as fp:
            conf = json.load(fp)
    return conf

conf = get_conf()

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

def gen_options_string(key, options, functor=None, shave_last=False, posix=False):
    OPTS_MAP = {
        'cflags': ' -%s',
        'preprocessor': ' -D%s',
        'inc_dir': ' -I"%s"' if not posix else ' -I%s',
        'ldflags': ' -%s',
        'lib_dir': ' -LIBPATH:"%s"' if not posix else ' -L%s',
        'dep_libs': ' %s',
        'srcs': '%s ',
        'objs': '%s.obj ' if not posix else '%s.o ',
    }

    if functor:
        options = ''.join((OPTS_MAP[key] % functor(opt) for opt in options))
    else:
        options = ''.join((OPTS_MAP[key] % opt for opt in options))
    if shave_last:
        options = options[:-1]

    return options

def gen_makefile_windows(pf, target):
    all_sources = get_sources_list(fullpath=False)
    mk_dict = dict(
        out=target,
        cflags=gen_options_string('cflags', conf.get('compile_options', [])),
        preprocessor=gen_options_string('preprocessor', conf.get('precompile_options', [])),
        inc_dir=gen_options_string('inc_dir', conf.get('inc_dir', [])),
        ldflags=gen_options_string('ldflags', conf.get('ldflags', [])),
        lib_dir=gen_options_string('lib_dir', conf.get('lib_dir', [])),
        dep_libs=gen_options_string('dep_libs', conf.get('dep_libraries', [])),
        srcs=gen_options_string('srcs', all_sources, shave_last=True),
        objs=gen_options_string('objs', all_sources, functor=lambda x: os.path.splitext(x)[0], shave_last=True)
    )
    return mk_dict

def gen_makefile_posix(pf, target):
    def get_posix_lib(lib):
        if lib.endswith('.a') or lib.endswith('.so'):
            return lib
        else:
            return lib.replace('lib', '-l')

    all_sources = get_sources_list(fullpath=True)
    mk_dict = dict(
        out=target,
        cflags=gen_options_string('cflags', conf.get('compile_options', []), posix=True),
        inc_dir=gen_options_string('inc_dir', conf.get('inc_dir', []), posix=True),
        ldflags=gen_options_string('ldflags', conf.get('ldflags', []), posix=True),
        lib_dir=gen_options_string('lib_dir', conf.get('lib_dir', []), posix=True),
        dep_libs=gen_options_string('dep_libs', conf.get('dep_libraries', []), functor=get_posix_lib, posix=True),
        srcs=gen_options_string('srcs', all_sources, shave_last=True, posix=True),
        objs=gen_options_string('objs', all_sources, functor=lambda x: os.path.splitext(x)[0], shave_last=True, posix=True)
    )
    return mk_dict

def gen_makefile(pf):
    GEN_FUNCTOR = {
        'Windows': gen_makefile_windows,
        'Linux': gen_makefile_posix,
        'Darwin': gen_makefile_posix
    }
    fun = GEN_FUNCTOR.get(pf)
    if not fun:
        return

    mk_dict = fun(pf, conf['out'])
    mk = None
    with open('./templates/{pf}/bin.mk'.format(pf=pf), 'r', encoding='utf-8') as rfp:
        mk = rfp.read().format(**mk_dict)
    with open('Makefile', 'w', encoding='utf-8') as wfp:
        mk and wfp.write(mk)

def clean_windows():
    if os.path.exists('Makefile'):
        subprocess.check_call('nmake clean', shell=True)
        subprocess.check_call('del Makefile', shell=True)

def clean_posix():
    if os.path.exists('Makefile'):
        subprocess.check_call('make clean', shell=True)
        subprocess.check_call('rm Makefile', shell=True)

def build():
    gen_makefile(platform.system())
    subprocess.check_call('nmake', shell=True)

def rebuild():
    clean()
    build()

def clean():
    GEN_FUNCTOR = {
        'Windows': clean_windows,
        'Linux': clean_posix,
        'Darwin': clean_posix
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
