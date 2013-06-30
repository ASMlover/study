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

Vector = {}

function Vector.new(size)
  local obj = {
    elems = {}, 
    len  = 0,
  }
  obj = newObject(obj, Vector)

  return obj
end


function Vector:size()
  return self.len
end

function Vector:empty()
  return self.len == 0
end

function Vector:push_back(x)
  self.elems[self.len + 1] = x
  self.len = self.len + 1
end 

function Vector:push_front(x) 
  for i = 2, self.len + 1 do 
    self.elems[i] = self.elems[i - 1]
  end 
  self.elems[1] = x 
  self.len = self.len + 1
end

function Vector:pop_back()
  if self.len == 0 then
    print('out of range')
    return
  end
  self.elems[self.len] = nil
  self.len = self.len - 1
end 

function Vector:pop_front() 
  for i = 1, self.len - 1 do 
    self.elems[i] = self.elems[i + 1]
  end 
  self.elems[self.len] = nil 
  self.len = self.len - 1
end

function Vector:front()
  return self.elems[1]
end 

function Vector:back()
  return self.elems[self.len]
end 

function Vector:set(i, x)
  if i <= 0 or i > self.len then
    print('out or range')
    return
  end 

  self.elems[i] = x
end 

function Vector:get(i)
  if i <= 0 or i > self.len then
    print('out or range')
    return
  end 

  return self.elems[i]
end 

function Vector:pairs()
  local i = 0
  return function()
    i = i + 1
    if i <= self.len then
      return self.elems[i]
    end
  end
end  

