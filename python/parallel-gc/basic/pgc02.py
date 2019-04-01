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

import gc
import os
import sys
import threading

class PGC(object):
    _instance = None

    def __init__(self):
        self.pipe_name = 'fifo_%s#%s' % (
                os.path.splitext(os.path.basename(sys.argv[0]))[0], os.getpid())

    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = PGC()
        return cls._instance

    def start(self):
        # gc.disable() don't work, some 3rd-party library will enable it back
        gc.set_threshold(0)
        if os.path.exists(self.pipe_name):
            os.unlink(self.pipe_name)
        os.mkfifo(self.pipe_name)
        rdthread = threading.Thread(target=self.rd_proc)
        rdthread.start()

        wrthread = threading.Thread(target=self.wr_proc)
        wrthread.start()

    def rd_proc(self):
        pass

    def wr_proc(self):
        pass

def main():
    pass

if __name__ == '__main__':
    main()
