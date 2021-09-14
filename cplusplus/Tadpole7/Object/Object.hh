// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <functional>
#include <vector>
#include <Common/Common.hh>

namespace Tadpole::Object {

enum class ObjType : u8_t {
  STRING,
  NATIVE,
  FUNCTION,
  UPVALUE,
  CLOSURE,
};

class BaseObject;
class StringObject;
class NativeObject;
class FunctionObject;
class UpvalueObject;
class ClosureObject;

using ObjectRef     = BaseObject*;
using ObjectVisitor = std::function<void (ObjectRef)>;

interface IObjectIterator : private Common::UnCopyable {
  virtual void iter_objects(ObjectVisitor&& visitor) = 0;
};

interface IChildrenIterator : private Common::UnCopyable {
  virtual void iter_children(ObjectVisitor&& visitor) = 0;
};

class BaseObject : private Common::UnCopyable {
  ObjType type_;
  bool is_marked_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() {}

  inline ObjType type() const noexcept { return type_; }
  inline bool is_marked() const noexcept { return is_marked_; }
  inline void set_marked(bool is_marked = true) noexcept { is_marked_ = is_marked; }

  virtual bool is_truthy() const { return true; }
  virtual str_t stringify() const { return "<object>"; }
  virtual void iter_children(ObjectVisitor&& visitor) {}

  const char* type_asstr() const noexcept;
  StringObject* as_string();
  const char* as_cstring();
  NativeObject* as_native();
  FunctionObject* as_function();
  UpvalueObject* as_upvalue();
  ClosureObject* as_closure();
};

}
