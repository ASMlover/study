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
#include <cstring>
#include <GC/GC.hh>
#include <Object/StringObject.hh>

namespace Tadpole::Object {

inline u32_t string_hash(const char* s, sz_t n) noexcept {
  u32_t hash = 2166136261u;
  for (sz_t i = 0; i < n; ++i) {
    hash ^= s[i];
    hash *= 16777619;
  }
  return hash;
}

StringObject::StringObject(const char* s, sz_t n, u32_t h, bool move_owner) noexcept
  : BaseObject(ObjType::STRING), size_(n), hash_(h) {
  if (move_owner) {
    data_ = const_cast<char*>(s);
  }
  else {
    data_ = new char[size_ + 1];
    std::memcpy(data_, s, n);
    data_[size_] = 0;
  }
}

StringObject::~StringObject() {
  delete [] data_;
}

bool StringObject::is_truthy() const {
  return size_ > 0;
}

str_t StringObject::stringify() const {
  return data_;
}

StringObject* StringObject::create(const char* s, sz_t n) {
  u32_t h = string_hash(s, n);
  if (auto* o = GC::GC::get_instance().get_interned(h); o != nullptr)
    return o;

  auto* o  = GC::make_object<StringObject>(s, n, h, false);
  GC::GC::get_instance().set_interned(h, o);

  return o;
}

StringObject* StringObject::concat(StringObject* s1, StringObject* s2) {
  sz_t n = s1->size() + s2->size();
  char* s = new char[n + 1];
  std::memcpy(s, s1->data(), s1->size());
  std::memcpy(s + s1->size(), s2->data(), s2->size());
  s[n] = 0;

  u32_t h = string_hash(s, n);
  if (auto* o = GC::GC::get_instance().get_interned(h); o != nullptr) {
    delete [] s;
    return o;
  }

  auto* o = GC::make_object<StringObject>(s, n, h, true);
  GC::GC::get_instance().set_interned(h, o);

  return o;
}

}
