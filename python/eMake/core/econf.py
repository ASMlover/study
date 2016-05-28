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

import json
from core import eutils
from core.eenv import MakeEnv

@eutils.singleton
class MakeConf(object):
    def __init__(self):
        self.conf = None
        self.obj_conf = None

    def load_conf(self, build_path):
        emake_dir = MakeEnv().get_emake_dir()
        try:
            with eutils.eopen('%s/conf.json' % build_path, 'r', encoding='utf-8') as fp:
                self.conf = json.load(fp)
        except Exception:
            conf_fname = '%s/templates/conf.%s.json' % (emake_dir, eutils.get_platform())
            with eutils.eopen(conf_fname, 'r', encoding='utf-8') as fp:
                self.conf = json.load(fp)

        obj_conf_fname = '%s/templates/obj.%s.mk' % (emake_dir, eutils.get_platform())
        with eutils.eopen(obj_conf_fname, 'r', encoding='utf-8') as fp:
            self.obj_conf = fp.read()

    def get_conf(self):
        assert self.conf is not None, 'Please load configure file first!!!'
        assert self.obj_conf is not None, 'Please load object file first!!!'
        return self.conf, self.obj_conf
