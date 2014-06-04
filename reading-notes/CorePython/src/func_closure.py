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

OUTPUT = '<int %r, id = %#0X, val = %d>'
w = x = y = z = 1

def f1():
  x = y = z = 2

  def f2():
    y = z = 3
  
    def f3():
      z = 4
      print OUTPUT % ('w', id(w), w)
      print OUTPUT % ('x', id(x), x)
      print OUTPUT % ('y', id(y), y)
      print OUTPUT % ('z', id(z), z)
  
    clo = f3.func_closure
    if clo:
      print "f3 closure vars:", [str(c) for c in clo]
    else:
      print "no f3 closure vars"
    f3()
  
  clo = f2.func_closure
  if clo:
    print "f2 closure vars:", [str(c) for c in clo]
  else:
    print "no f2 closure vars"
  f2()


clo = f1.func_closure
if clo:
  print 'f1 closure vars:', [str(c) for c in clo]
else:
  print "no f1 closure vars"
f1()
