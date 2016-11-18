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
import copy
import json
import os
import platform
import shutil
import subprocess
import sys

OUTDIR = 'build'

if sys.version_info.major < 3:
    import codecs
    def do_open(fname, mode='rb', encoding=None):
        return codecs.open(fname, mode, encoding)

    def do_iteritems(d, **kwargs):
        return d.iteritems(**kwargs)
else:
    do_open = open

    def do_iteritems(d, **kwargs):
        return iter(d.items(**kwargs))

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
    parser.add_argument('surtr_path', nargs='?', help='relative path of `Surtr`')
    args = parser.parse_args()
    return args.option, args.surtr_path

def merge_conf_with_default(custom_conf, default_conf):
    conf = copy.deepcopy(custom_conf)
    for key, val in do_iteritems(default_conf):
        if key not in conf:
            conf[key] = val
        else:
            if isinstance(val, dict):
                conf[key] = merge_conf_with_default(conf[key], val)
    return conf

def get_conf(tool_path='./', platform='linux'):
    ptname = 'posix'
    if platform == 'windows':
        ptname = 'windows'

    surtr_path = '{tool_path}/surtr/templates'.format(tool_path=tool_path)

    # get building Makefile template
    fname = '{surtr_path}/build.{pt}.mk'.format(surtr_path=surtr_path, pt=ptname)
    with do_open(fname, 'r', encoding='utf-8') as fp:
        build_mk = fp.read()

    # get object generation template
    fname = '{surtr_path}/obj.{pt}.mk'.format(surtr_path=surtr_path, pt=ptname)
    with do_open(fname, 'r', encoding='utf-8') as fp:
        obj_mk = fp.read()

    # get default build configure
    build_conf = {}
    fname = '{surtr_path}/default.{pt}.conf'.format(surtr_path=surtr_path, pt=ptname)
    with do_open(fname, 'r', encoding='utf-8') as fp:
        default_build_conf = json.load(fp)

    try:
        with do_open('surtr.conf', 'r', encoding='utf-8') as fp:
            build_conf = json.load(fp)

        build_conf = merge_conf_with_default(build_conf, default_build_conf)
    except Exception as e:
        build_conf = default_build_conf
    return build_mk, obj_mk, build_conf

def get_sources_for_dir(dirpath, valid_exts, recursive=True, exclusive=[]):
    cur_sources = os.listdir(dirpath)
    all_sources = []
    for source_name in cur_sources:
        source_fullpath = os.path.join(dirpath, source_name).replace('\\', '/')
        if os.path.isdir(source_fullpath) and recursive and source_name not in exclusive:
            all_sources.extend(get_sources_for_dir(source_fullpath, valid_exts, recursive, exclusive))
        else:
            if os.path.splitext(source_name)[1][1:] in valid_exts:
                all_sources.append(source_fullpath)
    return all_sources

def get_all_sources(sources_paths, valid_exts=('cpp', 'cc', 'cxx')):
    all_sources = []
    includes_dirs = sources_paths['includes']
    exclusive = sources_paths['exclusive']
    for sources_dir in includes_dirs:
        all_sources.extend(get_sources_for_dir(sources_dir['path'], valid_exts, sources_dir['recursive'], exclusive))
    return all_sources

def gen_outobj(source_fname, posix=True):
    s = source_fname.strip('./').strip('../').replace('/', '.')
    objname = os.path.splitext(s)[0]
    if posix:
        outobj_format = '$(OUTDIR)/$(OBJDIR)/{objname}.o '
    else:
        outobj_format = r'$(OUTDIR)\$(OBJDIR)\{objname}.obj '
    return outobj_format.format(objname=objname)

def gen_buildobj(conf, out, src):
    return conf.format(build_obj=out.strip(), build_src=src)

def gen_cflags(build_conf):
    cflags = []
    for option in build_conf['precompile_options']:
        cflags.append('-D{opt} '.format(opt=option))
    for option in build_conf['compile_options']:
        cflags.append('-{opt} '.format(opt=option))
    for option in build_conf['extra_options']:
        cflags.append('-{opt} '.format(opt=option))
    return ''.join(cflags).rstrip()

def gen_includes(build_conf, posix=True):
    includes = []
    for inc in build_conf['compile_includes']:
        if posix:
            includes.append('-I{inc} '.format(inc=inc))
        else:
            includes.append('-I"{inc}" '.format(inc=inc.replace('/', '\\')))
    return ''.join(includes).rstrip()

def gen_ldflags(build_conf, posix=True):
    ldincludes = []
    ldlibraries = []
    for inc in build_conf['link_includes']:
        if posix:
            ldincludes.append('-L{inc} '.format(inc=inc))
        else:
            ldincludes.append('-LIBPATH:"{inc}" '.format(inc=inc.replace('/', '\\')))

    if posix:
        for lib in build_conf['link_libraries']:
            if not lib.endswith('.a') and not lib.endswith('.so'):
                lib = lib.replace('lib', '-l')
            ldlibraries.append(lib)
    else:
        ldlibraries = build_conf['link_libraries']
    ldlibraries = ('{lib} '.format(lib=lib) for lib in ldlibraries)
    return '{incs}{libs}'.format(incs=''.join(ldincludes), libs=''.join(ldlibraries).rstrip())

def gen_makefile(surtr_path='./', platform='linux', outdir='build'):
    is_posix = platform != 'windows'
    build_mk, obj_mk, build_conf = get_conf(surtr_path, platform)
    sources = get_all_sources(build_conf['sources_paths'], valid_exts=build_conf['extensions'])

    objs_list = []
    buildobjs_list = []
    for s in sources:
        objstr = gen_outobj(s, is_posix)
        objs_list.append(objstr)
        buildobjs_list.append(gen_buildobj(obj_mk, objstr, s))
    build_dict = dict(
        outdir = outdir,
        target = build_conf['target'],
        cflags = gen_cflags(build_conf),
        includes = gen_includes(build_conf, is_posix),
        ldflags = gen_ldflags(build_conf, is_posix),
        objs = ''.join(objs_list).rstrip(),
        build_objs = ''.join(buildobjs_list).rstrip(),
    )

    with do_open('Makefile', 'w', encoding='utf-8') as fp:
        fp.write(build_mk.format(**build_dict))

def gen_outdir(outdir=OUTDIR):
    safe_mkdir(outdir)
    safe_mkdir('{outdir}/bin'.format(outdir=outdir))
    safe_mkdir('{outdir}/obj'.format(outdir=outdir))

def do_remove():
    safe_rm(OUTDIR)
    safe_rm('Makefile')

def main():
    option, surtr_path = get_options()
    if option == 'remove':
        do_remove()
        return

    if surtr_path is None:
        surtr_path = '../'

    platform = get_platform()
    gen_outdir()
    gen_makefile(surtr_path=surtr_path, platform=platform, outdir=OUTDIR)

    if option == 'config':
        return
    elif option is None:
        option = 'build'

    make = platform == 'windows' and 'nmake' or 'make'
    make_flags = {
        'build': '',
        'rebuild': 'rebuild',
        'clean': 'clean'
    }
    flag = make_flags.get(option)
    if flag is None:
        do_remove()
    else:
        subprocess.check_call('{make} {flag}'.format(make=make, flag=flag), shell=True)

if __name__ == '__main__':
    main()
