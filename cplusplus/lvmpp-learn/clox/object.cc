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

namespace clox {

template <typename Object, typename... Args> inline Object* make_object(Args&&... args) noexcept {
  Object* o = new Object(std::forward<Args>(args)...);
  get_vm().append_object(o);
  return o;
}

ObjString* Obj::as_string() noexcept {
  return as_down<ObjString>(this);
}

cstr_t Obj::as_cstring() noexcept {
  return as_down<ObjString>(this)->cstr();
}

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

}
