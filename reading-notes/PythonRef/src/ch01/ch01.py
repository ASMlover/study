#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2015 ASMlover. All rights reserved.
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

def file_reading():
    import sys
    file_obj = open('ch01.py')
    line = file_obj.readline()
    while line:
        sys.stdout.write(line)
        line = file_obj.readline()
    file_obj.close()

def echoing():
    while True:
        read_info = input('please input >>')
        if read_info == 'exit':
            break
        print (read_info)

def listing():
    import sys
    file_obj = open('ch01.py')
    [sys.stdout.write(line) for line in file_obj.readlines()]
    file_obj.close()

def yield_func():
    def counting(n):
        while n > 0:
            yield n
            n -= 1
    for c in counting(5):
        print (c)

def coroutine_func():
    def print_matches():
        while True:
            line = (yield)
            print (line)
    m = print_matches()
    m.__next__()
    while True:
        text = input('please input lines >>> ')
        if text == 'exit':
            m.close()
            break
        m.send(text)

def class_object():
    class stack(list):
        def push(self, x):
            self.append(x)

    s1 = stack()
    s1.push(1)
    s1.push(2)
    s1.push(3)
    print ('s1 => ', s1)

    s2 = stack()
    s2.push(6)
    s2.push(7)
    s2.push(8)
    print ('s2 => ', s2)

    s = s1 + s2
    print ('s => ', s)

def with_func():
    import sys
    file = open('ch01.py', 'r')
    with file:
        [sys.stdout.write(line) for line in file.readlines()]

if __name__ == '__main__':
    import sys

    def ch01_help():
        print ('********************* Help ********************')
        print ('1 => file reading sample')
        print ('2 => echo sample')
        print ('3 => list sample')
        print ('4 => yield sample')
        print ('5 => coroutine sample')
        print ('6 => class sample')
        print ('7 => with sample')
    if len(sys.argv) < 2:
        ch01_help()
        sys.exit()

    option = int(sys.argv[1])
    if option == 1:
        file_reading()
    elif option == 2:
        echoing()
    elif option == 3:
        listing()
    elif option == 4:
        yield_func()
    elif option == 5:
        coroutine_func()
    elif option == 6:
        class_object()
    elif option == 7:
        with_func()
    else:
        ch01_help()
