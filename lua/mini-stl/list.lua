-- Copyright (c) 2013 ASMlover. All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
--
--  * Redistributions of source code must retain the above copyright
--    notice, this list ofconditions and the following disclaimer.
--
--  * Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in
--    the documentation and/or other materialsprovided with the
--    distribution.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
-- FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
-- COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
-- INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
-- BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
-- CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
-- LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
-- ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
-- POSSIBILITY OF SUCH DAMAGE.

require 'object'


List = {}


function List.new()
  local obj = { head_ = {}, size_ = 0 }
  obj.head_.prev = obj.head_
  obj.head_.next = obj.head_

  obj = newObject(obj, List)
  return obj
end 

function List:clear()
  local it = self.head_.next
  while it ~= self.head_ do 
    local node = it 
    it = it.next
    self:erase(node)
  end 

  self.head_.prev = self.head_
  self.head_.next = self.head_
  self.size_ = 0
end 

function List:empty()
  return self.head_.next == self.head_
end 

function List:size()
  return self.size_
end 

function List:insert(pos, x)
  local node = { prev = nil, next = nil, data = x }
  node.prev = pos.prev
  node.next = pos 
  pos.prev.next = node 
  pos.prev = node 

  self.size_ = self.size_ + 1
end 

function List:erase(pos)
  local prev = pos.prev 
  local next = pos.next 
  prev.next = next 
  next.prev = prev 
  pos = nil 

  self.size_ = self.size_ - 1
end

function List:push_back(x)
  self:insert(self.head_, x)
end  

function List:push_front(x)
  self:insert(self.head_.next, x)
end 

function List:pop_back()
  self:erase(self.head_.prev)
end 

function List:pop_front()
  self:erase(self.head_.next)
end 

function List:front() 
  assert(self.head_.next ~= self.head_)
  local node = self.head_.next 
  return node.data
end 

function List:back()
  assert(self.head_.next ~= self.head_)
  local node = self.head_.prev
  return node.data
end

function List:pairs()
  local it = self.head_.next
  return function() 
    if it ~= self.head_ then
      local data = it.data 
      it = it.next 
      return data 
    end
  end
end 

