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



SingleList = {}



function SingleList.new()
  local obj = { front_ = nil, rear_ = nil, size_ = 0 }
  obj = newObject(obj, SingleList)

  return obj
end 


function SingleList:clear()
  while self.front_ ~= nil do 
    local node = self.front_
    self.front_ = self.front_.next 
    node = nil
  end 
  self.front_ = nil 
  self.rear_ = nil 
  self.size_ = 0
end 

function SingleList:empty()
  return self.front_ == nil
end 

function SingleList:size()
  return self.size_
end 

function SingleList:push_back(x)
  local node = { next = nil, data = x }

  if self.front_ == nil then 
    self.front_ = node 
    self.rear_ = self.front_ 
  else
    self.rear_.next = node 
    self.rear_ = node
  end 

  self.size_ = self.size_ + 1
end 

function SingleList:push_front(x)
  local node = { next = self.front_, data = x }
  self.front_ = node 
  if self.rear_ == nil then
    self.rear_ = node
  end
  self.size_ = self.size_ + 1
end 

function SingleList:pop_front()
  if self.front_ == nil then 
    return
  end 

  local node = self.front_
  self.front_ = self.front_.next
  node = nil 

  self.size_ = self.size_ - 1
end 

function SingleList:front()
  if self.front_ == nil then 
    return nil
  end 

  return self.front_.data 
end 

function SingleList:back()
  if self.rear_ == nil then 
    return nil
  end 

  return self.rear_.data
end 


function SingleList:pairs()
  local node = self.front_ 
  return function()
    if node ~= nil then 
      local data = node.data 
      node = node.next 
      return data
    end
  end
end 

