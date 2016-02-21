#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2016 ASMlover. All rights reserved.
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

import PathHelper as PH
PH.addPathes('../')

import datetime
import sys
import Timer
import traceback

import AsyncIo as asio

class IoService(object):
    def __init__(self, socketMap=None):
        super(IoService, self).__init__()
        self.stoped = False
        self.socketMap = socketMap
        self.profile = None
        self.profileName = None
        self.profileFlag = 0

        if sys.platform.startswith('linux'):
            import signal
            signal.signal(signal.SIGUSR1, self.profileHandler)
            signal.signal(signal.SIGINT, self.signalHandler)

    def poll(self):
        try:
            asio.loop(0, True, self.socketMap, 1)
        except:
            traceback.print_stack()

    def asyncLoop(self):
        asio.loop(0.01, True, self.socketMap, 1)

    def run(self, timeout=None):
        if timeout:
            Timer.addTimer(timeout, self.stop)
        while True:
            try:
                if self.profileFlag == 0:
                    self.asyncLoop()
                else:
                    if self.profile is None:
                        self.createProfile()
                    self.profile.runcall(self.asyncLoop)
            except KeyboardInterrupt:
                break
            except:
                # ignore all exceptions
                pass

            if self.stoped:
                break

        asio.closeAll()
        asio.loop(0.05, True, self.socketMap)

    def stop(self):
        self.stoped = True

    def createProfile(self):
        import cProfile
        self.profile = cProfile.Profile()
        dt = datetime.datetime.now()
        self.profileName = 'profile_%s_%s.prof' % (str(dt.date()), str(dt.time()))

    def signalHandler(self, signum, frame):
        self.stop()

    def profileHandler(self, signum, frame):
        if self.profileFlag == 0:
            self.profileFlag = 1
            self.createProfile()
        else:
            self.profileFlag = 0
            if self.profile is not None:
                self.profile.dump_stats(self.profileName)
            self.profile = None
            self.profileName = None
