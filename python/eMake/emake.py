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
from emake_env import MakeEnv
from core import eutils

def get_arguments():
    parser = argparse.ArgumentParser(description='C/C++ building tool')
    parser.add_argument('option', help='build|rebuild|clean the project')
    parser.add_argument('path', help='root path of the project', nargs='?', default='./')
    args = parser.parse_args()
    return args.option, args.path

def main():
    option, proj_path = get_arguments()
    MakeEnv().set_proj_path(proj_path)

    print('emake_dir=%s, proj_path=%s' % (MakeEnv().get_emake_dir(), MakeEnv().get_proj_path()))

    with eutils.eopen('README.md', 'r', encoding='utf-8') as fp:
        print(fp.read())

    all_sources = eutils.get_sources_list('./', exts=["py"], fullpath=True)
    for fname in all_sources:
        print(fname)

if __name__ == '__main__':
    main()
