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

import datetime
import platform
import traceback
import async_time as _at
import asyncore_scheduler as _as

class IoService(object):
    def __init__(self, socket_map=None):
        super(IoService, self).__init__()

        self._stopped = False
        self._socket_map = socket_map

        self._prof = None
        self._prof_name = None
        self._prof_started = False
        if platform.system() == 'Linux':
            import signal
            signal.signal(signal.SIGUSR1, self._profiler_handler)
            signal.signal(signal.SIGINT, self._sigint_handler)

    def _create_profiler(self):
        import cProfile
        self._prof = cProfile.Profile()
        dt = datetime.datetime.now()
        self._prof_name = 'NyxEngine.Profile.%s%s' % (dt.date(), dt.time())

    def _profiler_handler(self, signum, frame):
        if not self._prof_started:
            self._prof_started = True
            self._create_profiler()
        else:
            self._prof_started = False
            if self._prof is not None:
                self._prof.dump_stats(self._prof_name)
                self._prof = None
            self._prof_name = None

    def _sigint_handler(self, signum, frame):
        self.stop()

    def poll(self):
        try:
            _as.loop(0, True, self._socket_map, 1)
        except:
            traceback.print_stack()

    def async_loop(self):
        _as.loop(0.01, True, self._socket_map, 1)

    def run(self, timeout=None, process_large_buf=True):
        if timeout:
            _at.add_timer(timeout, self.stop)

        while True:
            try:
                if not self._prof_started:
                    self.async_loop()
                else:
                    if self._prof is None:
                        self._create_profiler()
                    self._prof.runcall(self.async_loop)
            except KeyboardInterrupt:
                break
            except:
                pass
            if self._stopped:
                break

        _as.close_all()
        _as.loop(0.05, True, self._socket_map)

    def stop(self):
        self._stopped = True
