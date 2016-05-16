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

def gen_makefile_windows(target, pf):
    # TODO: now just functional, need refactor
    cflags_list = ['O2', 'W3', 'MDd', 'GS', 'Zi', 'Fd"vc.pdb"', 'EHsc']
    cflags_list.extend(conf['compile_options'][pf])
    for opt in conf['precompile_options'][pf]:
        cflags_list.append('D%s' % opt)
    cflags_count = len(cflags_list)
    cflags_str = ''.join(('-%s ' % opt for opt in cflags_list))[:-1]

    ldflags_list = [
        'INCREMENTAL',
        'DEBUG',
        'PDB:$(TARGET).pdb',
        'manifest',
        'manifestfile:$(TARGET).manifest',
        'manifestuac:no'
    ]
    ldflags_list.extend(conf['link_options'][pf])
    extra_libs = conf['extra_libraries'][pf]
    ldflags_str = '%s%s' % (
            ''.join(('-%s ' % opt for opt in ldflags_list)),
            ''.join(('%s ' % lib for lib in extra_libs)))
    ldflags_str = ldflags_str[:-1]

    all_sources = get_sources_list()
    sources = ''.join(('%s ' % src for src in all_sources))[:-1]
    objs = ''.join(('%s.obj ' % os.path.splitext(src)[0]
                for src in all_sources))[:-1]

    with open('Makefile', 'w', encoding='utf-8') as pf:
        pf.write('TARGET = %s\n' % target)
        pf.write('RM = del\n')
        pf.write('CC = cl -c -nologo\n')
        pf.write('LINK = link -nologo\n')
        pf.write('MT = mt -nologo\n')
        pf.write('CFLAGS = %s\n' % cflags_str)
        pf.write('LDFLAGS = %s\n' % ldflags_str)
        pf.write('OBJS = %s\n' % objs)
        pf.write('SRCS = %s\n\n' % sources)
        pf.write('all: $(TARGET)\n\n')
        pf.write('rebuild: clean all\n\n')
        pf.write('clean:\n\t$(RM) $(TARGET) $(OBJS) *.pdb *.ilk *.manifest\n\n')
        pf.write('$(TARGET): $(OBJS)\n')
        pf.write('\t$(LINK) -out:$(TARGET) $(OBJS) $(LDFLAGS)\n')
        pf.write('\t$(MT) -manifest $(TARGET).manifest -outputresource:$(TARGET);1\n\n')
        pf.write('$(OBJS): $(SRCS)\n\t$(CC) $(CFLAGS) $(SRCS)')

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

    target = conf['target']
    GEN_FUNCTOR[pf](target, pf)

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
