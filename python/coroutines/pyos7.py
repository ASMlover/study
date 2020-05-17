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

import select
from collections import deque
from typing import Any, Deque, Dict, Generator, List, Optional, TextIO, Union

TaskGenerator = Generator[Any, Any, Any]

class SystemCall(object):
    def __init__(self) -> None:
        self.task: Optional[Task] = None
        self.sched: Optional[Scheduler] = None

    def handle(self) -> None:
        pass

class Task(object):
    taskid = 0

    def __init__(self, target: TaskGenerator) -> None:
        self.tid = self.gen_tid()
        self.target = target
        self.sendval: Optional[Any] = None

    def gen_tid(self) -> int:
        Task.taskid += 1
        return Task.taskid

    def run(self) -> Union[SystemCall, Any]:
        return self.target.send(self.sendval)

class Scheduler(object):
    def __init__(self) -> None:
        self.ready: Deque[Task] = deque()
        self.taskmap: Dict[int, Task] = {}
        self.exit_waiting: Dict[int, List[Task]] = {}
        self.read_waiting: Dict[int, Task] = {}
        self.write_waiting: Dict[int, Task] = {}

    def new(self, target: TaskGenerator) -> int:
        new_task = Task(target)
        self.taskmap[new_task.tid] = new_task
        self.schedule(task)
        return new_task.tid

    def schedule(self, task: Task) -> None:
        self.ready.append(task)

    def exit(self, task: Task) -> None:
        print(f"Task {task.tid} terminated")
        del self.taskmap[task.tid]

        for task in self.exit_waiting.pop(task.tid, []):
            self.schedule(task)

    def waitfor_exit(self, task: Task, waitid: int) -> bool:
        if waitid in self.taskmap:
            self.exit_waiting.setdefault(waitid, []).append(task)
            return True
        return False

    def waitfor_read(self, task: Task, fd: int) -> None:
        self.read_waiting[fd] = task

    def waitfor_write(self, task: Task, fd: int) -> None:
        self.write_waiting[fd] = task

    def iotask(self) -> TaskGenerator:
        while True:
            if not self.ready:
                self.iopoll(None)
            else:
                self.iopoll(0)
            yield

    def iopoll(self, timeout: Optional[float]):
        if self.read_waiting or self.write_waiting:
            r, w, e = select.select(self.read_waiting, self.write_waiting, [], timeout)
            for fd in r:
                self.schedule(self.read_waiting.pop(fd))
            for fd in w:
                self.schedule(self.write_waiting.pop(fd))

    def mainloop(self) -> None:
        self.new(self.iotask())
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
    def __init__(self, target: TaskGenerator) -> None:
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

class ReadWait(SystemCall):
    def __init__(self, f: TextIO) -> None:
        super(ReadWait, self).__init__()
        self.f = f

    def handle(self) -> None:
        fd = self.f.fileno()
        self.sched.waitfor_read(self.task, fd)

class WriteWait(SystemCall):
    def __init__(self, f: TextIO) -> None:
        super(WriteWait, self).__init__()
        self.f = f

    def handle(self) -> None:
        fd = self.f.fileno()
        self.sched.waitfor_write(self.task, fd)

if __name__ == '__main__':
    pass
