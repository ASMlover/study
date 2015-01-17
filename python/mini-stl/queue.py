#!/usr/bin/env python
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


class Queue(object):
  """ Queue of mini-stl """
  class QueueNode(object):
    def __init__(self):
      self.next = None
      self.data = None 
    def __del__(self):
      self.next = None 
      self.data = None 
  
  def __init__(self):
    self.front_ = None 
    self.rear_ = None 
  
  def __del__(self):
    self.clear()

  def clear(self):
    while self.front_ != None:
      node = self.front_
      self.front_ = self.front_.next 
      del node  
  
  def empty(self):
    return self.front_ == None 

  def push(self, x):
    node = self.QueueNode()
    node.next = None
    node.data = x 
    if self.front_ == None:
      self.front_ = node 
      self.rear_ = node 
    else:
      self.rear_.next = node 
      self.rear_ = node  

  def pop(self):
    assert self.front_ != None
    node = self.front_
    self.front_ = self.front_.next 
    del node  
 
  def top(self):
    assert self.front_ != None
    return self.front_.data 

