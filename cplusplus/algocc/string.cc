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
#include "string.hh"

namespace algocc {

String::String() noexcept {
}

String::~String() noexcept {
  if (data_ != nullptr)
    delete [] data_;
}

String::String(char c) noexcept {
  assign(c, 1);
}

String::String(char c, sz_t n) noexcept {
  assign(c, n);
}

String::String(const char* s) noexcept {
  if (s != nullptr)
    assign(s, std::strlen(s));
}

String::String(const char* s, sz_t n) noexcept {
  assign(s, n);
}

String::String(const String& s) noexcept {
  assign(s.data_, s.size_);
}

String::String(String&& s) noexcept {
  s.swap(*this);
}

String& String::operator=(const String& s) noexcept {
  if (this != &s)
    String(s).swap(*this);
  return *this;
}

String& String::operator=(String&& s) noexcept {
  if (this != &s)
    s.swap(*this);
  return *this;
}

void String::assign(char c, sz_t n) noexcept {
  if (n > 0) {
    size_ = n;
    data_ = new char[size_ + 1];
    std::memset(data_, c, size_);
    data_[size_] = 0;
  }
}

void String::assign(const char* s, sz_t n) noexcept {
  if (s != nullptr && n > 0) {
    size_ = n;
    data_ = new char[size_ + 1];
    std::memcpy(data_, s, size_);
    data_[size_] = 0;
  }
}

void String::swap(String& r) noexcept {
  std::swap(size_, r.size_);
  std::swap(data_, r.data_);
}

}
