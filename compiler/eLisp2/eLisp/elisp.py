#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2015 ASMlover. All rights reserved.
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

import getopt
import io
import pdb
import sys
from atom import TRUE, FALSE
from env import Environment
from error import Error
from fun import Function, Lambda, Closure
from lisp import Lisp
from reader import Reader

NAME = 'eLisp'
VERSION = 'v0.1'
PROMPT = '>>> '
DEPTH_MARK = '.'

class Elisp(Lisp):
    def __init__(self):
        self.stdin = sys.stdin
        self.stdout = sys.stdout
        self.stderr = sys.stderr

        self.debug = False
        self.verbose = True
        self.core = True
        self.closure = True

        self.reader = Reader()
        self.env = Environment()

        self.init()

    def init(self):
        # core functions
        self.env.set('eq',      Function(self.eq))
        self.env.set('quote',   Function(self.quote))
        self.env.set('car',     Function(self.car))
        self.env.set('cdr',     Function(self.cdr))
        self.env.set('cons',    Function(self.cons))
        self.env.set('atom',    Function(self.atom))
        self.env.set('cond',    Function(self.cond))

        # utility functions
        self.env.set('print',   Function(self.println))

        # special forms
        self.env.set('lambda',  Function(self.lambda_fun))
        self.env.set('label',   Function(self.label))

        # meta-elements
        self.env.set('__elisp__',   self)
        self.env.set('__global__',  self.env)

    def lambda_fun(self, env, args):
        if self.env != env.get('__global__') and self.closure:
            return Closure(env, args[0], args[1:])
        else:
            return Lambda(args[0], args[1:])

    def usage(self):
        self.print_banner()
        print ('%s <options> [elisp files]\n' % NAME.lower())

    def print_banner(self):
        print ('The %s programming shell %s' % (NAME, VERSION))
        print ('    Type `help` for more information\n')

    def print_help(self):
        print ('Help for eLisp %s' % VERSION)
        print ('    Type `help` for more information')
        print ('    Type `env` to see the bindings in current environment')
        print ('    Type `load` followed by one or more filenames to load source files')
        print ('    Type `quit` to exit the interpreter')

    def push(self, env=None):
        if env:
            self.env = self.env.push(env)
        else:
            self.env = self.env.push()

    def pop(self):
        self.env = self.env.pop()

    def repl(self):
        while True:
            source = self.get_complete_command()

            try:
                if source in ['quit']:
                    break
                elif source in ['help']:
                    self.print_help()
                elif source.startswith('load'):
                    files = source.split(' ')[1:]
                    self.process_files(files)
                elif source in ['env']:
                    print (self.env)
                else:
                    self.process(source)
            except AttributeError:
                print ('Could not process command: ', source)
                return

    def process(self, source):
        sexpr = self.reader.get_sexpr(source)

        while sexpr:
            result = None

            try:
                result = self.eval(sexpr)
            except Error as err:
                print (err)

            if self.verbose:
                self.stdout.write('     %s\n' % result)
            sexpr = self.reader.get_sexpr()

    def eval(self, sexpr):
        try:
            return sexpr.eval(self.env)
        except ValueError as err:
            print (err)
            return FALSE

    def get_complete_command(self, line="", depth=0):
        if line != '':
            line = line + ' '

        if self.env.level != 0:
            prompt = PROMPT + '%i%s ' % (self.env.level, DEPTH_MARK * (depth + 1))
        else:
            if depth == 0:
                prompt = PROMPT
            else:
                prompt = PROMPT + "%s " % (DEPTH_MARK * (depth + 1))
            line = line + self.read_line(prompt)

            balance = 0
            for c in line:
                if c == '(':
                    balance += 1
                elif c == ')':
                    balance -= 1
            if balance > 0:
                return self.get_complete_command(line, depth + 1)
            elif balance < 0:
                raise ValueError('Invalid paren pattern')
            else:
                return line

    def read_line(self, prompt):
        if prompt and self.verbose:
            self.stdout.write('%s' % prompt)
            self.stdout.flush()

        line = self.stdin.readline()
        if len(line) == 0:
            return 'EOF'

        if line [-1] == '\n':
            line = line[:-1]

        return line

    def process_files(self, files):
        self.verbose = False

        for filename in files:
            infile = open(filename, 'r')
            self.stdin = infile

            source = self.get_complete_command()
            while source not in ['EOF']:
                self.process(source)
                source = self.get_complete_command()
            infile.close()
        self.stdin = sys.stdin
        self.verbose = True


if __name__ == '__main__':
    elisp = Elisp()

    try:
        opts, files = getopt.getopt(sys.argv[1:], 
                'hd', ['help', 'debug', 'no-core', 'no-closures'])
    except getopt.GetoptError as err:
        print (str(err))
        elisp.usage()
        sys.exit(1)

    for opt, arg in opts:
        if opt in ('--help', '-h'):
            elisp.usage()
            sys.exit(1)
        elif opt in ('--debug', '-d'):
            elisp.verbose = True
        elif opt in ('--no-core'):
            elisp.core = False
        elif opt in ('--no-closures'):
            elisp.closure = False
        else:
            print ('unknown option ' + opt)

    if elisp.core:
        elisp.process_files(['core.el'])

    if len(files) > 0:
        elisp.process_files(files)

    elisp.print_banner()
    elisp.repl()
