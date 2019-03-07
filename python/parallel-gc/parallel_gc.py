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
import time
import json
import ctypes
import threading
import re
import sys

def singleton(cls, *args, **kw):
    instances = {}
    def _singleton():
        if cls not in instances:
            instances[cls] = cls(*args, **kw)
        return instances[cls]
    return _singleton

script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]

@singleton
class ParallelGC(object):
    def __init__(self):
        self.pipe_name = 'fifo_' + script_name #str(os.getpid())
        self.t0, self.t1, self.t2 = gc.get_threshold()
        self.use_threshold = False
        self.stop_pgc =False
        self.check_intvl = 300 # collect per 5 minutes

    def shutdown(self):
        self.stop_pgc = True

    def restart(self,check_intvl = 300):
        self.stop_pgc = False
        if self.stop_pgc is True:
            self.start_thread_collect(check_intvl)

    def set_intvl(self, check_intvl):
        self.check_intvl = check_intvl

    def gc_collect(self):
        # gc0 = gc.collect()
        gc.collect()
        list_grbg_id = []
        for item in gc.garbage:
            list_grbg_id.append(id(item))
        return list_grbg_id

    def set_threshold(self, t0=700, t1=10, t2=10):
        gc.set_threshold(t0,t1,t2)
        self.t0, self.t1, self.t2 = t0, t1, t2
        self.use_threshold = True

    def prcv(self):
        while 1:
            pipein = open(self.pipe_name, 'r')
            line = pipein.read()
            if line is not '[]' and line is not'[][]':
                line = re.search(r'(\[.*?\])+?$', line).group(1)
                try:
                    line = json.loads(line)
                except:
                     print "failed loads"
                list_grbg = []
                for grbg_id in line:
                    grbg = ctypes.cast(grbg_id, ctypes.py_object).value
                    list_grbg.append(grbg)
                for index in list_grbg:
                    if isinstance(index, list):
                        del index[:]
                    elif isinstance(index, dict):
                        index.clear()
                    else:
                        print index
                del list_grbg[:]
            time.sleep(self.check_intvl)
            pipein.close()
            if self.stop_pgc is True:
                break
        os.remove(self.pipe_name)
        os._exit(0)

    def prcv_once(self):
        pipein = open(self.pipe_name, 'r')
        line = pipein.read()
        if line is not '[]' and line is not'[][]':
            line = re.search(r'(\[.*?\])+?$', line).group(1)
            try:
                line = json.loads(line)
            except:
                 print "failed loads"
            list_grbg = []
            for grbg_id in line:
                grbg = ctypes.cast(grbg_id, ctypes.py_object).value
                list_grbg.append(grbg)
            for index in list_grbg:
                if isinstance(index, list):
                    del index[:]
                elif isinstance(index, dict):
                    index.clear()
                else:
                    print index
            del list_grbg[:]
        pipein.close()
        os.remove(self.pipe_name)

    def find_send(self):
        gc.set_debug(gc.DEBUG_SAVEALL)
        list_grbg_id = self.gc_collect()
        fd_out = os.open(self.pipe_name, os.O_WRONLY)
        s = json.dumps(list_grbg_id)
        os.write(fd_out, s)
        os.close(fd_out)
        os._exit(0)

    def fork_child(self):
        if self.use_threshold is True:
            gc.collect()
        while True:
            if self.use_threshold is True:
                c0, c1, c2 = gc.get_count()
                if c0 >= self.t0:
                    pid = os.fork()
                    if pid == 0:
                       self.find_send()
            else:
                pid = os.fork()
                if pid == 0:
                    self.find_send()
            time.sleep(self.check_intvl)
            if self.stop_pgc is True:
                break

    def fork_child_once(self):
        pid = os.fork()
        if pid == 0:
           self.find_send()

    def start_thread_collect(self, check_intvl):
        gc.disable()
        self.check_intvl = check_intvl
        if os.path.exists(self.pipe_name):
           os.unlink(self.pipe_name)
        os.mkfifo(self.pipe_name)
        rcv_thread = threading.Thread(target=self.prcv)
        rcv_thread.damen = True
        rcv_thread.start()

        fork_thread = threading.Thread(target=self.fork_child)
        fork_thread.damen = True
        fork_thread.start()

    def collect(self):
        gc.disable()
        if os.path.exists(self.pipe_name):
           os.unlink(self.pipe_name)
        os.mkfifo(self.pipe_name)
        self.fork_child_once()

        rcv_thread = threading.Thread(target=self.prcv_once)
        rcv_thread.damen = True
        rcv_thread.start()


pgc = ParallelGC()
def start(check_intvl):
    if pgc.stop_pgc is False:
        pgc.start_thread_collect(check_intvl)
    else:
        pgc.restart(check_intvl)

def collect():
    if pgc.stop_pgc is False:
        pgc.shutdown()
    pgc.collect()

def set_intvl(check_intvl):
    pgc.set_intvl(check_intvl)

def set_threshold(self, t0=700, t1=10, t2=10):
    pgc.set_threshold(t0, t1, t2)
