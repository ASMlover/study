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
import subprocess

def get_platform():
    return platform.system().lower()

def get_arguments():
    parser = argparse.ArgumentParser(description='devil building tool')
    parser.add_argument('option', help='build|rebuild|clean the project')
    args = parser.parse_args()
    return args.option

def main():
    pt = get_platform()
    make = 'make'
    if pt == 'windows':
        make = 'nmake'
        mkfile = 'Makefile.win.mk'
    elif pt == 'linux':
        mkfile = 'Makefile'
    elif pt == 'darwin':
        mkfile = 'Makefile.mac.mk'
    else:
        print ('>>>>>>>>>> do not support this platform >>>>>>>>>>')
        return

    option = get_arguments()
    base_cmd = '{make} -f {mkfile}'.format(make=make, mkfile=mkfile)
    if option == 'build':
        subprocess.check_call(base_cmd, shell=True)
    elif option == 'rebuild':
        subprocess.check_call('{base} rebuild'.format(base=base_cmd), shell=True)
    elif option == 'clean':
        subprocess.check_call('{base} clean'.format(base=base_cmd), shell=True)
    else:
        print ('>>>>>>>>>> unknown build command option >>>>>>>>>>')

if __name__ == '__main__':
    main()
