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

import pprint

_pp = pprint.PrettyPrinter(indent=2)

def _print_node(node, indent, indent_symbol):
    if isinstance(node, list):
        for child in node:
            for p in _print_node(child, indent, indent_symbol):
                yield p
    elif isinstance(node, int) or isinstance(node, float) or isinstance(node, str) or node is None:
        yield ' {}'.format(node)
    elif hasattr(node, 'fields'):
        yield '\n{}{}'.format(indent_symbol * indent, type(node).__name__)
        for field in node.fields:
            yield '\n{}{}:'.format(indent_symbol * (indent + 1), field, ':')
            for p in _print_node(getattr(node, field), indent + 2, indent_symbol):
                yield p
    else:
        yield '\nError! Unable to print {}'.format(node)

def print_ast(node, indent=0, indent_symbol=' ' * 4):
    print(''.join(_print_node(node, indent, indent_symbol)))

def print_tokens(tokens):
    _pp.pprint(tokens)

def print_env(env):
    _pp.pprint(env.as_dict())
