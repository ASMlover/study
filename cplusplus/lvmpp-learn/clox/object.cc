// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include "vm.hh"
#include "object.hh"
#include "chunk.hh"

namespace clox {

template <typename Object, typename... Args> inline Object* make_object(Args&&... args) noexcept {
  Object* o = new Object(std::forward<Args>(args)...);
  get_vm().append_object(o);
  return o;
}

// region <Obj>
ObjString* Obj::as_string() noexcept {
  return as_down<ObjString>(this);
}

cstr_t Obj::as_cstring() noexcept {
  return as_down<ObjString>(this)->cstr();
}

ObjFunction* Obj::as_function() noexcept {
  return as_down<ObjFunction>(this);
}

ObjNative* Obj::as_native() noexcept {
  return as_down<ObjNative>(this);
}

ObjClosure* Obj::as_closure() noexcept {
  return as_down<ObjClosure>(this);
}
// endregion <Obj>

// region <ObjString>
ObjString::ObjString(const char* chars, int length, u32_t hash) noexcept
  : Obj{ObjType::OBJ_STRING}, length_{length}, hash_{hash} {
  chars_ = new char[length_ + 1];
  std::memcpy(chars_, chars, length_);
  chars_[length_] = 0;
}

ObjString::~ObjString() {
  delete [] chars_;
}

bool ObjString::is_equal(Obj* r) const {
  if (this == r)
    return true;
  return is_same_type(this, r) ? is_equal_to(as_down<ObjString>(r)) : false;
}

str_t ObjString::stringify() const {
  return chars_;
}

ObjString* ObjString::create(const char* chars, int length) {
  u32_t hash = hash_string(chars, length);
  if (ObjString* interned = get_vm().get_interned(hash); interned != nullptr)
    return interned;

  ObjString* o = make_object<ObjString>(chars, length, hash);
  get_vm().set_interned(hash, o);
  return o;
}

ObjString* ObjString::concat(ObjString* a, ObjString* b) {
  int length = a->length() + b->length();
  char* chars = new char[length + 1];
  std::memcpy(chars, a->data(), a->length());
  std::memcpy(chars + a->length(), b->data(), b->length());
  chars[length] = 0;

  u32_t hash = hash_string(chars, length);
  if (ObjString* interned = get_vm().get_interned(hash); interned != nullptr) {
    delete [] chars;
    return interned;
  }

  ObjString* o = make_object<ObjString>(chars, length, hash);
  get_vm().set_interned(hash, o);
  return o;
}
// endregion <ObjString>

// region <ObjFunction>
ObjFunction::ObjFunction(int upvalue_count) noexcept
  : Obj{ObjType::OBJ_FUNCTION}, upvalue_count_{upvalue_count} {
  chunk_ = new Chunk{};
}

ObjFunction::~ObjFunction() {
  delete chunk_;
}

str_t ObjFunction::stringify() const {
  if (name_ == nullptr)
    return "<script>";
  return name_->stringify();
}

ObjFunction* ObjFunction::create(ObjString* name) {
  ObjFunction* o = make_object<ObjFunction>();
  return o;
}
// endregion <ObjFunction>

// region <ObjNative>
ObjNative::ObjNative(NativeFn&& function) noexcept
  : Obj{ObjType::OBJ_NATIVE}, function_{std::move(function)} {
}

ObjNative::~ObjNative() {
}

str_t ObjNative::stringify() const {
  return "<native fn>";
}

ObjNative* ObjNative::create(NativeFn&& function) {
  ObjNative* o = make_object<ObjNative>(std::move(function));
  return o;
}
// endregion <ObjNative>

// region <ObjClosure>
ObjClosure::ObjClosure(ObjFunction* function) noexcept
  : Obj{ObjType::OBJ_CLOSURE}, function_{function} {
}

ObjClosure::~ObjClosure() {
}

str_t ObjClosure::stringify() const {
  return function_->stringify();
}

ObjClosure* ObjClosure::create(ObjFunction* function) {
  ObjClosure* o = make_object<ObjClosure>(function);
  return o;
}
// endregion <ObjClosure>

// region <ObjUpvalue>
ObjUpvalue::ObjUpvalue(Value* slot) noexcept
  : Obj{ObjType::OBJ_UPVALUE}, location_{slot} {
}

ObjUpvalue::~ObjUpvalue() {
}

str_t ObjUpvalue::stringify() const {
  return "<upvalue>";
}

ObjUpvalue* ObjUpvalue::create(Value* slot) {
  ObjUpvalue* o = make_object<ObjUpvalue>(slot);
  return o;
}
// endregion <ObjUpvalue>

}
