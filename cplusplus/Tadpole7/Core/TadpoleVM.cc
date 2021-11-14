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
#include <cstdarg>
#include <iostream>
#include <Common/Colorful.hh>
#include <Core/Chunk.hh>
#include <GC/GC.hh>
#include <Object/NativeObject.hh>
#include <Core/CallFrame.hh>
#include <Compiler/Compiler.hh>
#include <Builtin/Builtins.hh>
#include <Core/TadpoleVM.hh>

namespace Tadpole::Core {

TadpoleVM::TadpoleVM() noexcept {
  gcompiler_ = new Compiler::GlobalCompiler();
  stack_.reserve(kDefaultCap);

  Builtin::register_builtins(*this);
  GC::GC::get_instance().append_roots("TadpoleVM", this);
}

TadpoleVM::~TadpoleVM() {
  delete gcompiler_;
  globals_.clear();
}

void TadpoleVM::define_native(const str_t& name, Value::TadpoleCFun&& fn) {
  globals_[name] = Object::NativeObject::create(std::move(fn));
}

InterpretRet TadpoleVM::interpret(const str_t& source_bytes) {
  Object::FunctionObject* fn = gcompiler_->compile(*this, source_bytes);
  if (fn == nullptr)
    return InterpretRet::ECOMPILE;

  // TODO:
  return InterpretRet::OK;
}

void TadpoleVM::iter_objects(Object::ObjectVisitor&& visitor) {
  // iterate Tadpole objects roots
  gcompiler_->iter_objects(std::move(visitor));
  for (auto& v : stack_)
    visitor(v.as_object(safe_t()));
  for (auto& f : frames_)
    visitor(f.closure());
  for (auto& g : globals_)
    visitor(g.second.as_object(safe_t()));
  for (auto* u = open_upvalues_; u != nullptr; u = u->next())
    visitor(u);
}

void TadpoleVM::reset() {
  stack_.clear();
  frames_.clear();
  open_upvalues_ = nullptr;
}

void TadpoleVM::runtime_error(const char* format, ...) {
  std::cerr << Common::Colorful::fg::red << "Traceback (most recent call last):" << std::endl;
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto& frame = *it;

    sz_t i = frame.frame_chunk()->offset_with(frame.ip()) - 1;
    std::cerr
      << "  [LINE: " << frame.frame_chunk()->get_line(i) << "] in "
      << "`" << frame.frame_fn()->name_asstr() << "()`"
      << std::endl;
  }

  va_list ap;
  va_start(ap, format);
  std::vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << Common::Colorful::reset << std::endl;

  reset();
}

void TadpoleVM::push(const Value::Value& value) noexcept {
  stack_.push_back(value);
}

Value::Value TadpoleVM::pop() noexcept {
  Value::Value value = stack_.back();
  stack_.pop_back();
  return value;
}

const Value::Value& TadpoleVM::peek(sz_t distance) const noexcept {
  return stack_[stack_.size() - 1 - distance];
}

}
