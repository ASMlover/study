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
from typing import Any, Deque, Dict, Generator, List, Optional, Union


TargetGenerator = Generator[Any, Any, Any]

class SystemCall(object):
    def __init__(self) -> None:
        self.task: Optional[Task] = None
        self.sched: Optional[Scheduler] = None

    def handle(self) -> None:
        pass

class Task(object):
    taskid = 0

    def __init__(self, target: TargetGenerator) -> None:
        Task.taskid += 1

        self.tid = Task.taskid
        self.target = target
        self.sendval = None

    def run(self) -> Union[SystemCall, Any]:
        return self.target.send(self.sendval)

class Scheduler(object):
    def __init__(self) -> None:
        self.ready: Deque[Task] = deque()
        self.taskmap: Dict[int, Task] = {}
        self.exit_waiting: Dict[int, List[Task]] = {}

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

        # notify other tasks waiting for exit
        for task in self.exit_waiting.pop(task.tid, []):
            self.schedule(task)

    def waitfor_exit(self, task: Task, waittid: int) -> bool:
        if waittid in self.taskmap:
            self.exit_waiting.setdefault(waittid, []).append(task)
            return True
        return False

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
    def handle(self) -> None:
        self.task.sendval = self.task.tid
        self.sched.schedule(self.task)

class NewTask(SystemCall):
    def __init__(self, target: TargetGenerator) -> None:
        super(NewTask, self).__init__()
        self.target = target

    def handle(self) -> None:
        tid = self.sched.new(self.target)
        self.task.sendval = tid
        self.sched.schedule(self.task)

class KillTask(SystemCall):
    def __init__(self, tid: int) -> None:
        super(KillTask, self).__init__()
        self.tid = tid

    def handle(self) -> None:
        task = self.sched.taskmap.get(self.tid)
        if task:
            task.target.close()
            self.task.sendval = True
        else:
            self.task.sendval = False
        self.sched.schedule(self.task)

class WaitTask(SystemCall):
    def __init__(self, tid: int) -> None:
        super(WaitTask, self).__init__()
        self.tid = tid

    def handle(self) -> None:
        result = self.sched.waitfor_exit(self.task, self.tid)
        self.task.sendval = result
        if not result:
            self.sched.schedule(self.task)

if __name__ == '__main__':
    def foo() -> TargetGenerator:
        for i in range(5):
            print(f"I'm foo {i}")
            yield

    def main() -> TargetGenerator:
        child = yield NewTask(foo())
        print(f"waiting for child")
        yield WaitTask(child)
        print(f"child done")

    sched = Scheduler()
    sched.new(main())
    sched.mainloop()
