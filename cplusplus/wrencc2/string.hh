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
#pragma once

#include "common.hh"

namespace wrencc {

class String final : public Copyable {
  static constexpr sz_t kCacheMask = 0x0f;

  sz_t size_;
  union {
    char cache_[kCacheMask + 1]{};
    char* data_;
  };

  inline bool __is_long(sz_t size) const noexcept { return ~kCacheMask & size; }
  inline bool __is_long() const noexcept { return ~kCacheMask & size_; }
  inline char* __get_data() noexcept { return __is_long() ? data_ : cache_; }
  inline const char* __get_data() const noexcept { return __is_long() ? data_ : cache_; }

  void destroy() noexcept {
    if (__is_long() && data_ != nullptr)
      delete [] data_;
  }

  void fill(char c, sz_t count = 1) noexcept {
    if (count < 1)
      return;

    size_ = count;
    if (__is_long())
      data_ = new char[size_ + 1];

    char* p = __get_data();
    std::memset(p, c, size_);
    p[size_] = 0;
  }

  void assign(const char* s) noexcept {
    if (s != nullptr)
      assign(s, std::strlen(s));
  }

  void assign(const char* s, sz_t n) noexcept {
    if (s == nullptr || n < 0)
      return;

    size_ = n;
    if (__is_long())
      data_ = new char[size_ + 1];

    char* p = __get_data();
    std::memcpy(p, s, size_);
    p[size_] = 0;
  }
public:
  String() noexcept {}
  ~String() noexcept { destroy(); }
  String(char c) noexcept { fill(c); }
  String(char c, sz_t count) noexcept { fill(c, count); }
  String(const char* s) noexcept { assign(s); }
  String(const char* s, sz_t n) noexcept { assign(s, n); }

  String(const String& s) noexcept {
    assign(s.data(), s.size());
  }

  String(String&& s) noexcept {
    std::swap(size_, s.size_);
    if (__is_long())
      std::swap(data_, s.data_);
    else
      std::swap(cache_, s.cache_);
  }

  String& operator=(const char* s) noexcept {
    destroy();
    assign(s);
    return *this;
  }

  String& operator=(const String& s) noexcept {
    if (this != &s) {
      destroy();
      assign(s.data(), s.size());
    }
    return *this;
  }

  String& operator=(String&& s) noexcept {
    if (this != &s) {
      destroy();

      std::swap(size_, s.size_);
      if (__is_long())
        std::swap(data_, s.data_);
      else
        std::swap(cache_, s.cache_);
    }
    return *this;
  }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline const char* cstr() const noexcept { return __get_data(); }
  inline const char* data() const noexcept { return __get_data(); }
  inline char at(sz_t i) const noexcept { return __get_data()[i]; }
  inline char operator[](sz_t i) const noexcept { return __get_data()[i]; }
};

}
