// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include "object.h"

namespace lox {

StringObject* Object::as_string(void) const {
  return dynamic_cast<StringObject*>(const_cast<Object*>(this));
}

const char* Object::as_cstring(void) const {
  return dynamic_cast<StringObject*>(const_cast<Object*>(this))->c_str();
}

StringObject* concat(const Object* x, const Object* y) {
  auto* a = x->as_string();
  auto* b = y->as_string();

  int length = a->length() + b->length();
  char* chars = new char[length + 1];
  std::memcpy(chars, a->c_str(), a->length());
  std::memcpy(chars + a->length(), b->c_str(), b->length());
  chars[length] = 0;

  auto* s = new StringObject();
  s->reset(chars, length);

  return s;
}

StringObject::StringObject(const std::string& s)
  : Object(ObjType::STRING)
  , length_(static_cast<int>(s.size()))
  , chars_(new char[length_ + 1]) {
  std::memcpy(chars_, s.c_str(), length_);
  chars_[length_] = 0;
}

StringObject::~StringObject(void) {
  if (chars_ != nullptr) {
    delete [] chars_;
    chars_ = nullptr;
    length_ = 0;
  }
}

StringObject::StringObject(const StringObject& s)
  : Object(ObjType::STRING)
  , length_(s.length_)
  , chars_(new char[length_ + 1]) {
  std::memcpy(chars_, s.chars_, length_);
  chars_[length_] = 0;
}

StringObject::StringObject(StringObject&& s)
  : Object(ObjType::STRING) {
  std::swap(length_, s.length_);
  std::swap(chars_, s.chars_);
}

StringObject& StringObject::operator=(const StringObject& s) {
  if (this != &s) {
    if (chars_ != nullptr)
      delete [] chars_;

    length_ = s.length_;
    chars_ = new char[length_ + 1];
    std::memcpy(chars_, s.chars_, length_);
    chars_[length_] = 0;
  }
  return *this;
}

StringObject& StringObject::operator=(StringObject&& s) {
  if (this != &s) {
    std::swap(length_, s.length_);
    std::swap(chars_, s.chars_);
  }
  return *this;
}

void StringObject::reset(const char* s, int n) {
  if (chars_ != nullptr)
    delete [] chars_;
  chars_ = const_cast<char*>(s);
  length_ = n;
}

bool StringObject::is_equal(const Object* o) const {
  auto* r = dynamic_cast<StringObject*>(const_cast<Object*>(o));
  if (this == r)
    return true;
  return r != nullptr && length_ == r->length_
    && std::memcmp(chars_, r->chars_, length_) == 0;
}

bool StringObject::is_truthy(void) const {
  return length_ != 0;
}

std::string StringObject::stringify(void) const {
  return chars_;
}

}
