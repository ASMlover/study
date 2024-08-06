#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright (c) 2024 ASMlover. All rights reserved.
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

import traceback

class PromiseScheduler(object):
    __slots__ = ('ready_queue', 'tick_no',)

    def __init__(self) -> None:
        super(PromiseScheduler, self).__init__()
        self.ready_queue = [] # type: List
        self.tick_no = 0 # type: int

    def post_schedule(self, promise, arg) -> None:
        self.ready_queue.append((promise, arg))

    def do_schedule(self) -> None:
        queue = self.ready_queue

        schedule_count = len(queue)
        for i in range(schedule_count):
            future, arg = queue[i]
            future.on_schedule(arg)
        del queue[:schedule_count]
        self.tick_no += 1

    def poll(self) -> None:
        if not self.ready_queue:
            return

        try:
            self.do_schedule()
        except Exception:
            traceback.print_exc()

default_scheduler = PromiseScheduler()
