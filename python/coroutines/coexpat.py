#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2020 ASMlover. All rights reserved.
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

import xml.parsers.expat
from typing import Generator, TextIO, Tuple, Union

YieldType = Tuple[str, Union[str, Tuple[str, dict]]]

def expat_parse(f: TextIO, target: Generator[YieldType, None, None]):
    parser = xml.parsers.expat.ParserCreate()
    parser.StartElementHandler = \
            lambda name, attrs: target.send(('start', (name, attrs)))
    parser.EndElementHandler = \
            lambda name: target.send(('end', name))
    parser.CharacterDataHandler = \
            lambda data: target.send(('content', data))
    # parser.ParseFile(f)
    parser.Parse(f.read())

if __name__ == '__main__':
    from buses import bus_locations, buses_to_dicts, filter_on_field

    expat_parse(open("demo.xml"),
            buses_to_dicts(
                filter_on_field("route", "147",
                    filter_on_field("direction", "North Bound",
                        bus_locations()))
        ))
