#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2017 ASMlover. All rights reserved.
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

import os
import pkgutil
import sys

import nyx_common as nc

def _nyx_get_module_names(module_dir):
    module_names = set()
    try:
        module_files = os.listdir(module_dir)
    except:
        print 'get module names failed for directory: %s' % module_dir
        return ()

    for fname in module_files:
        splited_list = fname.split('.')
        if len(splited_list) == 2:
            if splited_list[1] in ('py', 'pyc'):
                module_names.add(splited_list[0])
    module_names.discard('__init__')
    return module_names

def _nyx_get_modules(module_dir):
    module_names = _nyx_get_module_names(module_dir)
    modules = []
    nc.add_syspath(module_dir)
    for module_name in module_names:
        try:
            mod = __import__(module_name, fromlist=[''])
            if mod:
                modules.append(mod)
        except:
            print 'get module(%s) failed' % module_name

            import traceback
            traceback.print_exc()
            continue
    return modules

def _nyx_load_all_modules(dir_path):
    modules = []
    nc.add_syspath(dir_path)
    for importer, package_name, _ in pkgutil.walk_packages([dir_path]):
        if package_name not in sys.modules:
            mod = importer.find_module(package_name).load_module(package_name)
            modules.append(mod)
    return modules

if __name__ == '__main__':
    # TEST: for `_nyx_get_module_names`
    module_names = _nyx_get_module_names('.')
    print module_names

    # TEST: for `_nyx_get_modules`
    modules = _nyx_get_modules('.')
    print modules
