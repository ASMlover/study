# Copyright (c) 2014 ASMlover. All rights reserved.
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
#
#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import sys
import fileinput


def process(string):
  print 'Processing: ', string



def file_iter_by_ch(filename):
  f = open(filename)
  while True:
    char = f.read(1)
    if not char:
      break
    process(char)
  f.close()



def file_iter_by_line(filename):
  f = open(filename)
  while True:
    line = f.readline()
    if not line:
      break
    process(line)
  f.close()


def file_iter_by_ch_all(filename):
  f = open(filename)
  for char in f.read():
    process(char)
  f.close()


def file_iter_by_line_all(filename):
  f = open(filename)
  for line in f.readlines():
    process(line)
  f.close()


def file_iter_by_lazy(filename):
  for line in fileinput.input(filename):
    process(line)


def file_iter(filename):
  f = open(filename)
  for line in f:
    process(line)
  f.close()



if __name__ == '__main__':
  if len(sys.argv) < 2:
    print 'invalid arguments'
    exit(1)

  filename = 'file_iter.py'
  if sys.argv[1] == 'c':
    file_iter_by_ch(filename)
  elif sys.argv[1] == 'l':
    file_iter_by_line(filename)
  elif sys.argv[1] == 'ca':
    file_iter_by_ch_all(filename)
  elif sys.argv[1] == 'la':
    file_iter_by_line_all(filename)
  elif sys.argv[1] == 'lazy':
    file_iter_by_lazy(filename)
  else:
    file_iter(filename)
