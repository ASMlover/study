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

from __future__ import print_function

import os
import sys
from core import eutils
from core.econf import MakeConf
from core.eenv import MakeEnv

@eutils.singleton
class MakeShell(object):
    def _load_make_conf(self):
        conf, obj_conf = MakeConf().get_conf()
        library_attrs = conf.get('library', {})
        static = library_attrs.get('static', False)
        shared = library_attrs.get('shared', False)
        if static and shared:
            raise Exception('Cannot build static library and shared library at same time')
        return conf, obj_conf, static, shared

    def gen_sell(self):
        pf = eutils.get_platform()
        functor = getattr(self, '_gen_shell_%s' % pf, None)
        if not functor:
            raise Exception('No shell functor with platform "%s"' % pf)

        conf, obj_conf, static, shared = self._load_make_conf()
        mk_dict = functor(conf, obj_conf, static, shared)
        self._gen_makefile(pf, mk_dict, static, shared)

    def _get_template_file(self, pf, static=False, shared=False):
        fname_format = '{dir}/templates/{mk}.{pf}.mk'
        emake_dir = MakeEnv().get_emake_dir()
        if static:
            return fname_format.format(dir=emake_dir, mk='static', pf=pf)
        elif shared:
            return fname_format.format(dir=emake_dir, mk='shared', pf=pf)
        else:
            return fname_format.format(dir=emake_dir, mk='executable', pf=pf)

    def _gen_makefile(self, pf, mk_dict, static=False, shared=False):
        tempfile = self._get_template_file(pf, static, shared)
        mk = None
        with eutils.eopen(tempfile, 'r', encoding='utf-8') as rfp:
            mk = rfp.read().format(**mk_dict)
        with open('Makefile', 'w', encoding='utf-8') as wfp:
            mk and wfp.write(mk)

    def _gen_options(self, key, options, gen=None, shave_last=False, posix=False):
        MAPPER = {
            'cflags': ' -%s',
            'preprocessor': ' -D%s',
            'inc_dir': ' -I"%s"' if not posix else ' -I%s',
            'ldflags': ' -%s',
            'link_dir': ' -LIBPATH:"%s"' if not posix else ' -L%s',
            'link_libs': ' %s',
            'objs': '$(OUTOBJ)/%s.obj ' if not posix else '$(OUTOBJ)/%s.o ',
        }

        if gen:
            options = ''.join(MAPPER[key] % gen(opt) for opt in options)
        else:
            options = ''.join(MAPPER[key] % opt for opt in options)
        if shave_last:
            options = options[:-1]

        return options

    def _gen_objname(self, s):
        s = s.split('/')[-1]
        return os.path.splitext(s)[0]

    def _gen_shell_windows(self, conf, obj_conf, static=False, shared=False):
        all_sources = eutils.get_sources_list(MakeEnv().get_proj_path(), exts=conf['extensions'], fullpath=True)
        mk_dict = dict(
            target=conf['target'],
            cflags=self._gen_options('cflags', conf.get('compile_options', [])),
            preprocessor=self._gen_options('preprocessor', conf.get('precompile_options', [])),
            inc_dir=self._gen_options('inc_dir', conf.get('inc_dir', [])),
            objs=self._gen_options('objs', all_sources, gen=self._gen_objname, shave_last=True)
        )
        if not static:
            mk_dict['ldflags'] = self._gen_options('ldflags', conf.get('ldflags', []))
            mk_dict['link_dir'] = self._gen_options('link_dir', conf.get('link_dir', []))
            mk_dict['link_libs'] = self._gen_options('link_libs', conf.get('link_libs', []))

        objs = []
        for source in all_sources:
            obj_name = self._gen_options('objs', [source], gen=self._gen_objname, shave_last=True)
            if os.path.splitext(source)[1][1:] == 'c':
                clang = '-EHs'
            else:
                clang = '-EHsc'
            objs.append(obj_conf.format(mk_obj=obj_name, mk_src=source, clang=clang))
        mk_dict['emake_objs'] = ''.join(objs)
        return mk_dict

    def _gen_shell_linux(self, conf, obj_conf, static=False, shared=False):
        mk_dict = self._gen_shell_linux(conf, obj_conf, static, shared)
        ldflags = mk_dict['ldflags']
        if not shared and ldflags.startswith(' '):
            mk_dict['ldflags'] = ldflags[1:]
        return mk_dict

    def _gen_shell_darwin(self, conf, obj_conf, static=False, shared=False):
        return self._gen_shell_linux(conf, obj_conf, static, shared)

    def _gen_shell_posix_impl(self, conf, obj_conf, static=False, shared=False):
        def _gen_library(lib):
            if lib.endswith('.a') or lib.endswith('.so'):
                return lib
            else:
                return lib.replace('lib', '-l')

        all_sources = eutils.get_sources_list(MakeEnv().get_proj_path(), exts=conf['extensions'], fullpath=True)
        mk_dict = dict(
            target=conf['target'],
            cflags=self._gen_options('cflags', conf.get('compile_options', [])),
            inc_dir=self._gen_options('inc_dir', conf.get('inc_dir', []), posix=True),
            objs=self._gen_options('objs', all_sources, gen=self._gen_objname, shave_last=True, posix=True)
        )
        if not static:
            mk_dict['ldflags'] = self._gen_options('ldflags', conf.get('ldflags', []), posix=True)
            mk_dict['link_dir'] = self._gen_options('link_dir', conf.get('link_dir', []), posix=True)
            mk_dict['link_libs'] = self._gen_options('link_libs', conf.get('link_libs', []), gen=_gen_library, posix=True)

        objs = []
        for source in all_sources:
            obj_name = self._gen_options('objs', [source], gen=self._gen_objname, shave_last=True, posix=True)
            objs.append(obj_conf.format(mk_obj=obj_name, mk_src=source))
        mk_dict['emake_objs'] = ''.join(objs)
        return mk_dict
