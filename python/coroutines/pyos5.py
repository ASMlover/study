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

from collections import deque
from typing import Any, Deque, Dict, Generator


TargetGenerator = Generator[Any, Any, Any]

class Task(object):
    taskid = 0

    def __init__(self, target: TargetGenerator):
        Task.taskid += 1
        self.tid = Task.taskid
        self.target = target
        self.sendval = None

    def run(self) -> Any:
        return self.target.send(self.sendval)

class SystemCall(object):
    def handle(self):
        pass

class Scheduler(object):
    def __init__(self):
        self.ready: Deque[Task] = deque()
        self.taskmap: Dict[int, Task] = {}

    def new(self, target: TargetGenerator) -> int:
        new_task = Task(target)
        self.taskmap[new_task.tid] = new_task
        self.schedule(new_task)
        return new_task.tid

    def schedule(self, task: Task) -> None:
        self.ready.append(task)

    def exit(self, task: Task) -> None:
        print(f"Task {task.tid} terminated")
        del self.taskmap[task.tid]

    def mainloop(self) -> None:
        while self.taskmap:
            task = self.ready.popleft()
            try:
                result = task.run()
                if isinstance(result, SystemCall):
                    result.task = task
                    result.sched = self
                    result.handle()
                    continue
            except StopIteration:
                self.exit(task)
                continue
            self.schedule(task)

class GetTid(SystemCall):
    def handle(self):
        self.task.sendval = self.task.tid
        self.sched.schedule(self.task)

class NewTask(SystemCall):
    def __init__(self, target: TargetGenerator):
        self.target = target

    def handle(self):
        tid = self.sched.new(self.target)
        self.task.sendval = tid
        self.sched.schedule(self.task)

class KillTask(SystemCall):
    def __init__(self, tid):
        self.tid = tid

    def handle(self):
        task = self.sched.taskmap.get(self.tid)
        if task:
            task.target.close()
            self.task.sendval = True
        else:
            self.task.sendval = False
        self.sched.schedule(self.task)

if __name__ == '__main__':
    def foo():
        mytid = yield GetTid()
        while True:
            print(f"I'm foo, {mytid}")
            yield

    def main():
        child = yield NewTask(foo())
        for _ in range(5):
            yield
        yield KillTask(child)
        print(f"main done")

    sched = Scheduler()
    sched.new(main())
    sched.mainloop()
