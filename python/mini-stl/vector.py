#!/usr/bin/python -e
# -*- encoding: utf-8 -*-
#
# Copyright (c) 2013 ASMlover. All rights reserved.
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

class Vector(object):
  """ Vector of mini-stl """
  def __init__(self):
    self.elems_ = []
    self.size_ = 0
    self.it = -1

  def __del__(self):
    self.elems_ = []
    self.size_ = 0 
  
  def __iter__(self):
    return self

  def next(self):
    self.it += 1
    if self.it == self.size_:
      raise StopIteration
    else:
      return self.elems_[self.it]

  def clear(self):
    self.elems_ = []

  def empty(self):
    return 0 == self.size_

  def size(self):
    return self.size_

  def push_back(self, v):
    self.elems_.append(v)
    self.size_ += 1

  def pop_back(self):
    if self.size_ != 0:
      del self.elems_[self.size_ - 1]
      self.size_ -= 1
  
  def __getitem__(self, i):
    assert i >= 0 and i < self.size_
    return self.elems_[i]

  def __setitem__(self, i, v):
    assert i >= 0 and i < self.size_
    self.elems_[i] = v 

  def front(self):
    assert self.size_ > 0
    return self.elems_[0]

  def back(self):
    assert self.size_ > 0
    return self.elems_[self.size_ - 1] 

  def for_each(self, visit):
    assert visit
    for e in self.elems_:
      visit(e)
