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
import tyr_interpreter as interpreter

tyr:
    input = raw_input
except NameError:
    pass

def parse_args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('-v', '--verbose', action='store_true')
    argparser.add_argument('file', nargs='?')
    return argparser.parse_args()

def interpret_file(path, verbose=False):
    with open(path) as f:
        print (interpreter.evaluate(f.read(), verbose=verbose))

def repl():
    print ('TYR 0.1. Press Ctrl+C to exit.')
    env = interpreter.create_global_env()
    buf = ''
    try:
        while True:
            inp = input('>>> ' if not buf else '')
            if inp == '':
                print (interpreter.evaluate_env(buf, env))
                buf = ''
            else:
                buf += '\n' + inp
    except KeyboardInterrupt:
        pass

def main():
    args = parse_args()
    if args.file:
        interpret_file(args.file, args.verbose)
    else:
        repl()

if __name__ == '__main__':
    main()
