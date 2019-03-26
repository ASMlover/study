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

def parent(pipe_name):
    fdin = open(pipe_name, 'r')
    line = fdin.read()
    objlists = json.loads(line)
    for oid in objlists:
        o = ctypes.cast(oid, ctypes.py_object).value
        print '>>>>>>>>> garbage object: ', o
        if isinstance(o, list):
            del o[:]
        elif isinstance(o, dict):
            o.clear()
        else:
            print o
    del objlists[:]
    fdin.close()
    sys.exit(0)

def child(pipe_name):
    gc.set_debug(gc.DEBUG_SAVEALL)
    gc.collect()
    garbage = [id(x) for x in gc.garbage]
    del gc.garbage[:]

    fdout = os.open(pipe_name, os.O_WRONLY)
    s = json.dumps(garbage)
    os.write(fdout, s)
    os.close(fdout)
    sys.exit(0)

def main():
    pipe_name = 'pgc_%s_%s' % (
            os.path.splitext(os.path.basename(sys.argv[0]))[0], os.getpid())
    if os.path.exists(pipe_name):
        os.unlink(pipe_name)
    os.mkfifo(pipe_name)

    l = []
    l.append(l)
    del l

    gc.disable()
    pid = os.fork()
    if pid != 0:
        parent(pipe_name)
    else:
        child(pipe_name)

if __name__ == '__main__':
    main()
