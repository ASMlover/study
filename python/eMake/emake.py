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

import argparse
import os
import shutil
import subprocess
import sys
from core import eutils
from core.econf import MakeConf
from core.ecore import MakeShell
from core.eenv import MakeEnv

def get_arguments():
    parser = argparse.ArgumentParser(description='C/C++ building tool')
    parser.add_argument('option', help='build|rebuild|clean|remove the project')
    args = parser.parse_args()
    return args.option

def build(make):
    MakeShell().gen_sell()
    subprocess.check_call(make, shell=True)

def rebuild(make):
    if os.path.exists('Makefile'):
        cmd = '%s rebuild' % make
        subprocess.check_call(cmd, shell=True)

def clean(make):
    if os.path.exists('Makefile'):
        cmd = '%s clean' % make
        subprocess.check_call(cmd, shell=True)

def remove():
    build_path = MakeEnv().get_build_path()
    if os.path.exists(build_path):
        shutil.rmtree(build_path)

def main():
    option = get_arguments()
    MakeEnv().set_env()
    MakeConf().load_conf()

    if option == 'remove':
        remove()
    else:
        mk = 'nmake' if eutils.get_platform() == 'windows' else 'make'
        fun = getattr(sys.modules['__main__'], option, None)
        os.chdir(MakeEnv().get_build_path())
        fun and fun(mk)
        os.chdir('..')

if __name__ == '__main__':
    main()
