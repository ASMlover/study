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

import os
import traceback

from MarsLog.LogManager import LogManager
import PathHelper as PH

_logger = LogManager.getLogger('Utils.EntityScanner')

def _getModuleFiles(moduleDir):
    moduleNameSet = set()
    try:
        files = os.listdir(moduleDir)
    except:
        _logger.error('error in _getModuleFiles: directory is %s', moduleDir)
        return ()
    for filename in files:
        fileList = filename.split('.')
        if len(fileList) == 2:
            moduleName, extension = fileList
            if extension in ('py', 'pyc'):
                moduleNameSet.add(moduleName)
    moduleNameSet.discard('__init__')
    return moduleNameSet

def _getModuleList(moduleDir):
    moduleNameSet = _getModuleFiles(moduleDir)
    moduleList = []
    PH.addPath(moduleDir)
    for moduleName in moduleNameSet:
        try:
            module = __import__(moduleName, fromlist=[''])
            module and moduleList.append(module)
        except:
            _logger.error('error in _getModuleList: %s', moduleName)
            traceback.print_exc()
            continue
    _logger.info('_getModuleList: %s', str(moduleList))
    return moduleList

def _getClassList(module, entityBaseClass):
    classList = []
    for name in dir(module):
        attr = getattr(module, name)
        if isinstance(attr, type) and issubclass(attr, entityBaseClass):
            classList.append(attr)
    return classList

def scanEntityClasses(moduleDir, entityBaseClass):
    classDict = {}
    moduleList = _getModuleList(moduleDir)
    for module in moduleList:
        classList = _getClassList(module, entityBaseClass)
        for cls in classList:
            classDict[cls.__name__] = cls
    return classDict
