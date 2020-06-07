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
import types
from collections import deque
from typing import Any, Deque, Dict, Generator, List, Optional, TextIO, Union


DispGenerator = Generator[Any, Any, Any]

class SystemCall(object):
    def __init__(self) -> None:
        self.task = None # type: Optional[Task]
        self.sched = None # type: Optional[Scheduler]

    def handle(self) -> None:
        pass

class Task(object):
    taskid = 0 # type: int

    def __init__(self, target: DispGenerator) -> None:
        self.tid = self.gen_tid()
        self.target = target
        self.sendval : Optional[Any] = None
        self.stack: List[DispGenerator] = []

    def gen_tid(self) -> int:
        Task.taskid += 1
        return Task.taskid

    def run(self) -> Union[SystemCall, Any]:
        pass

class Scheduler(object):
    def __init__(self) -> None:
        self.ready_queue: Deque[Task] = deque()
        self.taskmap: Dict[int, Task] = {}
        self.exit_waiting: Dict[int, List[Task]] = {}
        self.read_waiting: Dict[int, Task] = {} # reading waiting list
        self.writ_waiting: Dict[int, Task] = {} # writing waiting list
        self.expt_waiting: Dict[int, Task] = {} # exceptional waiting list

    def create(self, target: DispGenerator) -> int:
        new_task = Task(target)
        self.taskmap[new_task.tid] = new_task
        self.schedule(new_task)
        return new_task.tid

    def schedule(self, task: Task) -> None:
        self.ready_queue.append(task)

    def get_task(self, tid: int) -> Optional[Task]:
        return self.taskmap.get(tid)

    def exit(self, task: Task) -> None:
        tid = task.tid
        print(f"[Scheduler.exit] Task `{tid}` terminated")

        del self.taskmap[tid]

        for task in self.exit_waiting.pop(tid, []):
            self.schedule(task)

    def waitfor_exit(self, task: Task, waitid: int) -> bool:
        if waitid in self.taskmap:
            self.exit_waiting.setdefault(waitid, []).append(task)
            return True
        return False

    def waitfor_read(self, task: Task, fd: int) -> None:
        self.read_waiting[fd] = task

    def waitfor_write(self, task: Task, fd: int) -> None:
        self.writ_waiting[fd] = task

    def waitfor_except(self, task: Task, fd: int) -> None:
        self.expt_waiting[fd] = task

    def io_task(self) -> DispGenerator:
        while True:
            if not self.ready_queue:
                self.io_poll(None)
            else:
                self.io_poll(0)
            yield

    def io_poll(self, timeout: Optional[float]) -> None:
        if self.read_waiting or self.writ_waiting:
            r, w, e = select.select(self.read_waiting,
                    self.writ_waiting, self.expt_waiting, timeout)
            for fd in r:
                self.schedule(self.read_waiting.pop(fd))
            for fd in w:
                self.schedule(self.writ_waiting.pop(fd))
            for fd in e:
                self.schedule(self.expt_waiting.pop(fd))

    def main_loop(self) -> None:
        self.create(self.io_task())
        while self.taskmap:
            task = self.ready_queue.popleft()
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
    def __init__(self, target: DispGenerator) -> None:
        super(NewTask, self).__init__()
        self.target = target

    def handle(self) -> None:
        tid = self.sched.create(self.target)
        self.task.sendval = tid
        self.sched.schedule(self.task)

class KillTask(SystemCall):
    def __init__(self, tid: int) -> None:
        super(KillTask, self).__init__()
        self.tid = tid

    def handle(self) -> None:
        task = self.sched.get_task(self.tid)
        if task:
            task.target.close()
            self.task.sendval = True
        else:
            self.task.sendval = False
        self.sched.schedule(self.task)

def __get_tid() -> SystemCall:
    return GetTid()

def __create_task(target: DispGenerator) -> SystemCall:
    return NewTask(target)

def __kill_task(tid: int) -> SystemCall:
    return KillTask(tid)

__CALLMAP = {
    "gettid": __get_tid,
    "newtask": __create_task,
    "killtask": __kill_task,
}
def syscall(func_name: str, *args, **kwds) -> Optional[SystemCall]:
    func = __CALLMAP.get(func_name)
    if func:
        return func(*args, **kwds)
    return None
