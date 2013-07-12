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

class List(object):
  """ List module for mini-stl(python)
      same as std::list<T> in C++ stl
  """
  class ListNode(object):
    def __init__(self):
      self.prev = None
      self.next = None 
      self.data = None 
    def __del__(self):
      self.prev = None
      self.next = None
      self.data = None 

  def __init__(self):
    self.head_ = self.ListNode()
    self.head_.prev = self.head_ 
    self.head_.next = self.head_
    self.size_ = 0 
    self.iter_ = None
 
  def __del__(self):
    self.clear()
    self.iter_ = None 
 
  def __iter__(self):
    self.iter_ = self.begin()
    return self

  def next(self):
    if self.iter_ == self.end():
      raise StopIteration
    else:
      data = self.iter_.data 
      self.iter_ = self.iter_.next
      return data

  def clear(self):
    it = self.begin()
    while it != self.end():
      node = it 
      it = it.next 
      self.erase(node)
    self.head_.prev = self.head_
    self.head_.next = self.head_
    self.size_ = 0

  def empty(self):
    return self.begin() == self.end()

  def size(self):
    return self.size_ 

  def insert(self, pos, x):
    node = self.ListNode()
    node.data = x 
    node.prev = pos.prev
    node.next = pos 
    pos.prev.next = node 
    pos.prev = node 

    self.size_ += 1 

  def erase(self, pos):
    prev = pos.prev 
    next = pos.next 
    prev.next = next 
    next.prev = prev 
    del pos 

    self.size_ -= 1
 
  def begin(self):
    return self.head_.next

  def end(self):
    return self.head_
  
  def push_front(self, x):
    self.insert(self.begin(), x)

  def push_back(self, x):
    self.insert(self.end(), x) 

  def pop_front(self):
    assert self.begin() != self.end()
    self.erase(self.front())
  
  def pop_back(self):
    assert self.begin() != self.end()
    pos = self.end()
    self.erase(pos.prev)
  
  def front(self):
    assert self.begin() != self.end()
    return self.begin().data

  def back(self):
    assert self.begin() != self.end()
    return self.end().prev.data  

  def for_each(self, visit):
    assert visit
    node = self.begin()
    while node != self.end():
      visit(node.data)
      node = node.next
