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

import ctypes
import gc
import json
import os
import sys
import threading

def child(pipe_name):
    gc.set_debug(gc.DEBUG_SAVEALL)
    gc.collect()
    objlist = [id(o) for o in gc.garbage]
    del gc.garbage[:]

    fdwrite = os.open(pipe_name, os.O_WRONLY)
    os.write(fdwrite, json.dumps(objlist))
    os.close(fdwrite)
    sys.exit(-1)

def parent(pipe_name):
    fdread = open(pipe_name, 'r')
    objlist = json.loads(fdread.read())
    for objid in objlist:
        o = ctypes.cast(objid, ctypes.py_object).value
        print '********* collect garbage object: ', o
        if isinstance(o, list):
            del o[:]
        elif isinstance(o, dict):
            o.clear()
        else:
            print o
    del objlist[:]
    fdread.close()
    os.remove(pipe_name)

def collect():
    gc.disable()

    pipe_name = 'fifo_%s#%s' % (
            os.path.splitext(os.path.basename(sys.argv[0]))[0], os.getpid())
    if os.path.exists(pipe_name):
        os.unlink(pipe_name)
    os.mkfifo(pipe_name)

    pid = os.fork()
    if pid == 0:
        child(pipe_name)

    t = threading.Thread(target=parent, args=(pipe_name,))
    t.start()
    t.join()

def main():
    l = []
    l.append(l)
    d = {}
    d[1] = l
    d[2] = d

    del l
    del d
    collect()

if __name__ == '__main__':
    main()
