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
import platform
import subprocess
import sys

def get_arguments():
    parser = argparse.ArgumentParser(description='Building tools for Tyr')
    parser.add_argument('option', help='make|clean the project')
    args = parser.parse_args()
    return args.option

def get_platform():
    return platform.system().lower()

def make(makefile):
    cmd = 'make -f {fname}'.format(fname=makefile)
    subprocess.check_call(cmd, shell=True)

def clean(makefile):
    cmd = 'make -f {fname} clean'.format(fname=makefile)
    subprocess.check_call(cmd, shell=True)

def main():
    pt = get_platform()
    if pt == 'linux':
        makefile = 'make.linux.mk'
    elif pt == 'darwin':
        makefile = 'make.darwin.mk'
    else:
        print ('********** INVALID PLATFORM (support Linux/macOS only) **********')
        sys.exit()

    option = get_arguments()
    fun = getattr(sys.modules['__main__'], option, None)
    fun and fun(makefile)

if __name__ == '__main__':
    main()
