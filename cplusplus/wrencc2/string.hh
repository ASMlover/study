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

  sz_t size_{};
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

  String& operator=(char c) noexcept {
    destroy();
    fill(c);
    return *this;
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

  inline bool operator==(char c) const noexcept { return compare(c); }
  inline bool operator!=(char c) const noexcept { return !compare(c); }
  inline bool operator==(const char* s) const noexcept { return compare(s); }
  inline bool operator!=(const char* s) const noexcept { return !compare(s); }
  inline bool operator==(const String& s) const noexcept { return compare(s); }
  inline bool operator!=(const String& s) const noexcept { return !compare(s); }
  inline String& operator+=(char c) noexcept { return append(c); }
  inline String& operator+=(const char* s) noexcept { return append(s); }
  inline String& operator+=(const String& s) noexcept { return append(s); }
  inline String operator+(char c) const noexcept { return join(c); }
  inline String operator+(const char* s) const noexcept { return join(s); }
  inline String operator+(const String& s) const noexcept { return join(s); }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline const char* cstr() const noexcept { return __get_data(); }
  inline const char* data() const noexcept { return __get_data(); }
  inline char at(sz_t i) const noexcept { return __get_data()[i]; }
  inline char operator[](sz_t i) const noexcept { return __get_data()[i]; }

  inline bool compare(char c) const noexcept {
    return size_ == 1 && __get_data()[0] == c;
  }

  inline bool compare(const char* s, sz_t n) const noexcept {
    return size_ == n && std::memcmp(__get_data(), s, size_) == 0;
  }

  inline bool compare(const char* s) const noexcept {
    return s != nullptr && compare(s, std::strlen(s));
  }

  inline bool compare(const String& s) const noexcept {
    return this == &s || compare(s.data(), s.size());
  }

  String& append(char c) noexcept {
    sz_t old_size = size_;
    sz_t new_size = old_size + 1;

    if (__is_long(new_size)) {
      char* new_data = new char[new_size + 1];
      std::memcpy(new_data, __get_data(), old_size);

      destroy();
      data_ = new_data;
    }
    size_ = new_size;

    char* p = __get_data();
    p[old_size] = c;
    p[size_] = 0;

    return *this;
  }

  String& append(const char* s, sz_t n) noexcept {
    if (s == nullptr || n <= 0)
      return *this;

    sz_t old_size = size_;
    sz_t new_size = old_size + n;

    if (__is_long(new_size)) {
      char* new_data = new char[new_size + 1];
      std::memcpy(new_data, __get_data(), old_size);

      destroy();
      data_ = new_data;
    }
    size_ = new_size;

    char* p = __get_data();
    std::memcpy(p + old_size, s, n);
    p[size_] = 0;

    return *this;
  }

  String& append(const char* s) noexcept {
    if (s != nullptr)
      append(s, std::strlen(s));
    return *this;
  }

  String& append(const String& s) noexcept {
    return append(s.data(), s.size());
  }

  inline String join(char c) const noexcept {
    return String(*this).append(c);
  }

  inline String join(const char* s, sz_t n) const noexcept {
    return String(*this).append(s, n);
  }

  inline String join(const char* s) const noexcept {
    return String(*this).append(s);
  }

  inline String join(const String& s) const noexcept {
    return String(*this).append(s);
  }

  String slice(sz_t start, sz_t count) const {
    if (start >= size_ || count == 0)
      return String();

    if (start + count > size_)
      count = size_ - start;
    return String(__get_data() + start, count);
  }

  int find(const String& sub, int start) const {
    if (sub.empty())
      return start;
    if (start >= size_ || start + sub.size() > size_)
      return -1;

    const char* found = std::strstr(__get_data() + start, sub.data());
    return found != nullptr ? Xt::as_type<int>(found - __get_data()) : -1;
  }

  static String from_decimal(double d) noexcept {
    std::stringstream ss;
    ss << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
    return String(ss.str().data());
  }
};

inline std::ostream& operator<<(std::ostream& o, const String& s) {
  return o << s.cstr();
}

}
