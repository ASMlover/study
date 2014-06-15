# Copyright (c) 2014 ASMlover. All rights reserved.
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
#
#!/usr/bin/env python
# -*- encoding: utf-8 -*-


from random import randint
from time import sleep
from Queue import Queue
from myThread import MyThread 



def WriteQueue(queue):
  print 'producing object for Queue ...', queue.put('xxx', 1)
  print 'size now', queue.qsize()


def ReadQueue(queue):
  val = queue.get(1)
  print 'consumed object from Queue ... size now', queue.qsize()


def Writer(queue, loops):
  for i in range(loops):
    WriteQueue(queue)
    sleep(randint(1, 3))


def Reader(queue, loops):
  for i in range(loops):
    ReadQueue(queue)
    sleep(randint(2, 5))



FUNCS = [Writer, Reader]
NFUNCS = range(len(FUNCS))



def Main():
  nloops = randint(2, 5)
  q = Queue(32)

  threads = []
  for func in FUNCS:
    t = MyThread(func, (q, nloops), func.__name__)
    threads.append(t)
  
  for i in NFUNCS:
    threads[i].start()

  for i in NFUNCS:
    threads[i].join()

  print 'all DONE'




if __name__ == '__main__':
  Main()
