-- Copyright (c) 2014 ASMlover. All rights reserved.
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


-- Extending for standard library of Lua.


function printf(format, ...)
  print(string.format(tostring(format), ...))
end




-- Checking.
-- Checking and transfering to number, return 0 if failed.
function CheckNumber(value, base)
  return tonumber(value, base) or 0
end

-- Checaking and transfering to integer, return 0 if failed.
function CheckInt(value)
  return math.round(CheckNumber(value))
end

-- Checking and transfering to boolean, return true except nil and false.
function CheckBool(value)
  return (value ~= nil and value ~= false)
end

-- Checking wether it's table, return a nil table if it's not a table.
function CheckTable(value)
  if type(value) ~= 'table' then
    value = {}
  end

  return value
end

-- Return false while hashtable[key] is nil or hashtable is not a table.
function IsSet(hashtable, key)
  local t = type(hashtable)
  return (t == 'table' or t = 'userdata') and hashtable[key] ~= nil
end




-- Deep clone.
function DeepClone(object)
  local lookup_table = {}
  local function Copy(object)
    if type(object) ~= 'table' then
      return object
    elseif lookup_table[object] then
      return lookup_table[object]
    end

    local new_table = {}
    lookup_table[object] = new_table
    for key, value in pairs(object) do 
      new_table[Copy(key)] = Copy(value)
    end

    return setmetatable(new_table, getmetatable(object))
  end

  return Copy(object)
end



-- Creating a class.
--
-- Defining a base class.
--    local Shape = class('Shape')
--
-- Constructor function Ctor(), it will be called by Shape.New()
--    function Shape:Ctor(shape_name)
--      self.shape_name_ = shape_name
--      printf('Shape:Ctor(%s)', self.shape_name_)
--    end
--
--    function Shape:Draw()
--      printf('Shape:Draw %s', self.shape_name_)
--    end
--
--
--  Circle is child of Shape.
--    local Circle = class('Circle', Shape)
--
--    function Circle:Ctor()
--      -- Must call Base's Ctor by yourself when it override base's Ctor.
--      Circle.super.Ctor(self, 'Circle')
--      self.radius_ = 100
--    end
--
--    function Circle:SetRadius(radius)
--      self.radius_ = radius
--    end
--
--    function Circle:Draw()
--      printf('Circle:Draw %s, radius = %f', 
--        self.shape_name_, self.radius_)
--    end
function class(classname, super)
  local super_type = type(super)
  local cls

  if super_type ~= 'function' and super_type ~= 'table' then
    super_type = nil
    super = nil
  end

  if super_type == 'function' or (super and super.__ctype == 1) then
    -- inherited from native C++ object.
    cls = {}

    if super_type == 'table' then
      -- copy fields from super
      for key, value in pairs(super) do 
        cls[key] = value
      end
      cls.__create = super.__create
      cls.super    = super
    else
      cls.__create = super
      cls.Ctor     = function() end
    end

    cls.__cname = classname
    cls.__ctype = 1

    function cls.New(...)
      local instance = cls.__create(...)
      -- copy fields from class to native object
      for key, value in pairs(cls) do 
        instance[key] = value
      end
      instance.class = cls
      instance:Ctor(...)

      return instance
    end
  else 
    -- inherited from lua object
    if super then
      cls = {}
      setmetatable(cls, {__index = super})
      cls.super = super
    else 
      cls = {Ctor = function() end}
    end

    cls.__cname = classname
    cls.__ctype = 2 -- Lua
    cls.__index = cls

    function cls.New(...)
      local instance = setmetatable({}, cls)
      instance.class = cls
      instance:Ctor(...)

      return instance
    end
  end

  return cls
end
