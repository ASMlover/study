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

#include <vector>
#include <unordered_map>
#include <Common/Common.hh>
#include <Object/Value.hh>

namespace Tadpole::Core {

enum class InterpretRet : u8_t {
  OK,
  ECOMPILE, // compile error
  ERUNTIME, // runtime error
};

class CallFrame;
class GlobalCompiler;

class TadpoleVM final : public Object::IObjectIterator {
  static constexpr sz_t kDefaultCap = 256;

  GlobalCompiler* gcompiler_{};
  bool is_running_{true};

  std::vector<Value::Value> stack_;
  std::vector<CallFrame> frames_;

  std::unordered_map<str_t, Value::Value> globals_;
  Object::UpvalueObject* open_upvalues_{};

  virtual void iter_objects(Object::ObjectVisitor&& visitor) override;

  void reset();
  void runtime_error(const char* format, ...);

  void push(const Value::Value& value) noexcept;
  Value::Value pop() noexcept;
  const Value::Value& peek(sz_t distance = 0) const noexcept;

  bool call(Object::ClosureObject* closure, sz_t nargs);
  bool call(const Value::Value& callee, sz_t nargs);
  Object::UpvalueObject* capture_upvalue(Value::Value* local);
  void close_upvalues(Value::Value* last);

  InterpretRet run();
public:
  TadpoleVM() noexcept;
  virtual ~TadpoleVM();

  inline bool is_running() const noexcept { return is_running_; }
  inline void terminate() noexcept { is_running_ = false; }


  void define_native(const str_t& name, Value::TadpoleCFun&& fn);
  InterpretRet interpret(const str_t& source_bytes);
};

}
