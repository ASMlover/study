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

from __future__ import print_function
import time

class Toggle(object):
    def __init__(self, start_state):
        self.state = start_state

    def value(self):
        return self.state

    def activate(self):
        self.state = not self.state
        return self

class NthToggle(Toggle):
    def __init__(self, start_state, max_counter):
        super(NthToggle, self).__init__(start_state)
        self.count_max = max_counter
        self.count = 0

    def activate(self):
        self.count += 1
        if self.count >= self.count_max:
            super(NthToggle, self).activate()
            self.count = 0
        return self

def main():
    start = time.time()
    n = 100000
    val = True
    toggle = Toggle(val)

    for i in range(0, n):
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
      val = toggle.activate().value()
    print(toggle.value())

    val = True
    ntoggle = NthToggle(val, 3)

    for i in range(0, n):
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
      val = ntoggle.activate().value()
    print(ntoggle.value())

    print("use: %s" % (time.time() - start))

if __name__ == '__main__':
    main()
