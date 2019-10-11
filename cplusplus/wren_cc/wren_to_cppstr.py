#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2019 ASMlover. All rights reserved.
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

PREAMBLE="""// generated automatically from {0}, do not edit

static const str_t kLibSource =
{1};
"""

def wren_to_cppstr(input_path, wren_source_lines):
    wren_source = ''
    for line in wren_source_lines:
        line = line.replace('"', '\\\"')
        line = line.replace('\n', '\\n\"')
        if wren_source:
            wren_source += '\n'
        wren_source += '"' + line

    return PREAMBLE.format(input_path, wren_source)

def main():
    parser = argparse.ArgumentParser(
            description='convert a Wren library to a C++ string literal')
    parser.add_argument('input', help='the source `*.wren` file')
    parser.add_argument('output', help='the output file to write')

    args = parser.parse_args()

    with open(args.input, 'r') as fp:
        wren_source_lines = fp.readlines()

    cpp_source = wren_to_cppstr(args.input, wren_source_lines)
    with open(args.output, 'w') as fp:
        fp.write(cpp_source)

if __name__ == '__main__':
    # DEMO: $ python wren_to_cppstr.py builtin/core.wren core.wren.hh
    main()
