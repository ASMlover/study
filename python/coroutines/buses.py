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

import xml.sax
import coroutine
from typing import Generator, Tuple, Union

YieldType = Tuple[str, Union[str, Tuple[str, dict]]]


class EventHandler(xml.sax.ContentHandler):
    def __init__(self, target: Generator[YieldType, None, None]) -> None:
        self.target = target

    def startElement(self, name: str, attrs: xml.sax.xmlreader.AttributesImpl) -> None:
        self.target.send(('start', (name, attrs._attrs)))

    def characters(self, content: str) -> None:
        self.target.send(('content', content))

    def endElement(self, name: str) -> None:
        self.target.send(('end', name))

@coroutine.corouine
def buses_to_dicts(target: Generator[dict, None, None]) \
        -> Generator[YieldType, None, None]:
    while True:
        event, value = (yield)
        if event == 'start' and value[0] == 'bus':
            busdict = {}
            fragments = []
            while True:
                event, value = (yield)
                if event == 'start':
                    fragments = []
                elif event == 'content':
                    fragments.append(value)
                elif event == 'end':
                    if value != 'bus':
                        busdict[value] = ''.join(fragments)
                    else:
                        target.send(busdict)
                        break

@coroutine.corouine
def filter_on_field(fieldname: str, value: str, target: Generator[dict, None, None]) \
        -> Generator[dict, None, None]:
    while True:
        d = (yield)
        if d.get(fieldname) == value:
            target.send(d)

@coroutine.corouine
def bus_locations() -> Generator[dict, None, None]:
    while True:
        bus = (yield)
        print(f"{bus['route']}, {bus['id']}, \"{bus['direction']}\", "
                f"{bus['latitude']}, {bus['longitude']}")

if __name__ == '__main__':
    xml.sax.parse("demo.xml", EventHandler(
        buses_to_dicts(
            filter_on_field("route", "147",
                filter_on_field("direction", "North Bound",
                    bus_locations()))
        )))
