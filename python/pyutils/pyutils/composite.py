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

import sys
import inspect

def _formatted_path(path, components):
    if not path:
        return components

    result = []
    for comp in components:
        result.append('.'.join([path, comp]))
    return result

def _gen_sequence_function(funcs):
    def _caller(*args, **kwargs):
        for f in funcs:
            f(*args, **kwargs)
    return _caller

def _extract_component_units(component, properties, funcs):
    """gather properties and functions of component"""
    for name, attr in component.iteritems():
        if name.startswith('__') and name.endswith('__') and name != '__init__':
            continue
        if not (inspect.ismethod(attr) or inspect.isfunction(attr)
                or inspect.ismethoddescriptor(attr)):
            assert name not in properties, 'name is: %s' % name
            properties[name] = attr
            continue

        methods = funcs.setdefault(name, [])
        methods.append(attr)

def _gather_component_units(components, properties, funcs):
    """gather all properties and functions of all components"""
    for comp in components:
        token = comp.split('.')
        module_name = '.'.join(token[:-1])
        component_name = token[-1]

        __import__(module_name)
        module = sys.modules[module_name]
        comp = getattr(module, component_name)
        _extract_component_units(comp.__dict__, properties, funcs)

def _gather_function(properties, funcs):
    """generate functions of all components as sequence"""
    for name, fun in funcs.iteritems():
        if len(fun) == 1:
            properties[name] = fun[0]
        else:
            properties[name] = _gen_sequence_function(fun)

def gather_components(attrs):
    components = attrs.get('_components')
    if not components:
        # TODO: deal with special attrs
        return attrs

    path = attrs.get('_component_path')
    components = _formatted_path(path, components)

    funcs = {}
    properties = {}
    _extract_component_units(attrs, properties, funcs)
    properties.update(attrs)

    _gather_component_units(components, properties, funcs)
    _gather_function(properties, funcs)

    # TODO: deal with special attrs
    return properties

class CompositeAssemble(type):
    def __new__(cls, name, bases, dct):
        properties = gather_components(dct)
        return super(CompositeAssemble, cls).__new__(cls, name, bases, properties)
