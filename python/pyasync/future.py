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

import sys
import traceback
from enum import IntEnum
from typing import Any, Callable
from .throwables import CancelledError, InvalidState, PromiseReject
from .scheduler import default_scheduler as scheduler

class JobState(IntEnum):
    PENDING = -1
    REJECTED = 0
    FULLFILLED = 1
    CANCELLED = 2

class ExceptionWrap(object):
    __slots__ = ('etype', 'value', 'tb',)

    def __init__(self, info: tuple) -> None:
        super(ExceptionWrap, self).__init__()

        if info is None:
            info = sys.exc_info()
        self.etype, self.value, _ = info
        self.tb = None

    def __repr__(self) -> str:
        return ''.join(traceback.format_exception(self.etype, self.value, self.tb))

    def reraise(self) -> None:
        tb, self.tb = self.tb, None
        raise self.etype, self.value, tb

    def call_excepthook(self) -> None:
        tb, self.tb = self.tb, None
        sys.excepthook(self.etype, self.value, tb)

class Future(object):
    __slots__ = (
        'state',
        'scheduler',
        'on_fullfilled_callback',
        'on_rejected_callback',
        'on_fullfilled',
        'on_rejected',
        'post_fired',
        'result',
        'exc_handled',
        'origin',
        'upstream',
    )

    def __init__(self) -> None:
        super(Future, self).__init__()
        self.state = JobState.PENDING
        self.on_fullfilled_callback = None
        self.on_rejected_callback = None
        self.on_fullfilled = []
        self.on_rejected = []
        self.post_fired = False
        self.result = None
        self.exc_handled = None
        self.origin = None
        self.upstream = None

    def __repr__(self) -> str:
        return f'<Future({self.state.name}: {self.result}) at {id(self):#X}>'

    @staticmethod
    def internal_create(on_fullfilled: Callable, on_rejected: Callable, origin: tuple) -> Future:
        r = Future().set_upstream(*origin)
        r.on_fullfilled_callback = on_fullfilled
        r.on_rejected_callback = on_rejected
        return r

    @staticmethod
    def resolve(value: Any) -> Future:
        future = Future()
        future.on_resolve(value)
        return future

    @staticmethod
    def reject(value: Any) -> Future:
        future = Future()
        future.on_reject(value)
        return future

    @property
    def state(self) -> JobState:
        return self.state

    @property
    def is_pending(self) -> bool:
        return self.state == JobState.PENDING

    @property
    def is_fullfilled(self) -> bool:
        return self.state == JobState.FULLFILLED

    @property
    def is_rejected(self) -> bool:
        return self.state in (JobState.REJECTED, JobState.CANCELLED)

    @property
    def is_cancelled(self) -> bool:
        return self.state == JobState.CANCELLED

    @property
    def is_settled(self) -> bool:
        return self.state != JobState.PENDING

    def set_result(self, value: Any = None) -> None:
        if self.is_settled:
            raise InvalidState("Future alreay settled.")
        self.on_resolve(value)

    def try_set_result(self, value: Any = None) -> None:
        if self.is_settled:
            return
        self.on_resolve(value)

    def set_exception(self, value: Any) -> None:
        if self.is_settled:
            raise InvalidState("Future alreay settled.")
        self.on_reject(value)

    def try_set_exception(self, value: Any) -> None:
        if self.is_settled:
            return
        self.on_reject(value)

    def try_cancel(self) -> bool:
        if self.is_settled:
            return False
        self.on_reject(CancelledError())
        return True

    def on_resolve(self, result: Any = None) -> None:
        self.set_state(JobState.FULLFILLED, result)

    def on_reject(self, result: Any) -> bool:
        pass

    def set_state(self, state: JobState, result: Any) -> None:
        if not self.is_pending:
            return

        self.state = state
        self.result = result
        if state == JobState.FULLFILLED:
            del self.on_rejected[::]
            if self.on_fullfilled:
                self.post_callback()
        else:
            del self.on_fullfilled[::]
            if self.on_rejected:
                self.post_callback()

    def post_callback(self) -> None:
        if self.is_pending:
            return

        if not self.post_fired:
            self.post_fired = True
            scheduler.post_schedule(self, 0)

    def set_upstream(self, upstream, method) -> Future:
        return self

    def add_fullfilled_callback(self, callback: Callable) -> None:
        if self.state == JobState.REJECTED:
            return

        self.on_fullfilled.append(callback)
        self.post_callback()

    def add_rejected_callback(self, callback: Callable) -> None:
        if self.state == JobState.FULLFILLED:
            return
        self.on_rejected.append(callback)
        self.post_callback()
