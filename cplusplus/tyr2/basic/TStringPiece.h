// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_BASIC_STRINGPIECE_HEADER_H__
#define __TYR_BASIC_STRINGPIECE_HEADER_H__

#include <string.h>
#include <string>
#include "TTypes.h"

namespace tyr { namespace basic {

class StringArg {
  const char* str_;
public:
  StringArg(const char* s)
    : str_(s) {
  }

  StringArg(const std::string& s)
    : str_(s.c_str()) {
  }

  const char* c_str(void) const {
    return str_;
  }
};

class StringPiece {
  const char* str_;
  size_t length_;
public:
  StringPiece(void)
    : str_(nullptr)
    , length_(0) {
  }

  StringPiece(const char* s)
    : str_(s)
    , length_(strlen(str_)) {
  }

  StringPiece(const byte_t* s)
    : str_(reinterpret_cast<const char*>(s))
    , length_(strlen(str_)) {
  }

  StringPiece(const std::string& s)
    : str_(s.data())
    , length_(s.size()) {
  }

  StringPiece(const char* s, size_t n)
    : str_(s)
    , length_(n) {
  }

  explicit operator bool(void) const {
    return nullptr != str_;
  }

  const char* data(void) const {
    return str_;
  }

  char operator[](std::ptrdiff_t i)  const {
    return str_[i];
  }

  size_t size(void) const {
    return length_;
  }

  bool empty(void) const {
    return 0 == length_;
  }

  const char* begin(void) const {
    return str_;
  }

  const char* end(void) const {
    return str_ + length_;
  }

  void clear(void) {
    str_ = nullptr;
    length_ = 0;
  }

  void set(const char* s) {
    str_ = s;
    length_ = strlen(str_);
  }

  void set(const char* s, size_t n) {
    str_ = s;
    length_ = n;
  }

  void set(const void* buf, int len) {
    str_ = reinterpret_cast<const char*>(buf);
    length_ = len;
  }

  void remove_prefix(size_t n) {
    str_ += n;
    length_ -= n;
  }

  void remove_suffix(size_t n) {
    length_ -= n;
  }

  std::string as_string(void) const {
    return std::string(data(), size());
  }

  void copy_to_string(std::string* x) const {
    x->assign(str_, length_);
  }

  bool starts_with(const StringPiece& x) const {
    return ((length_ >= x.length_) && (0 == memcmp(str_, x.str_, x.length_)));
  }

  int compare(const StringPiece& x) const {
    int r = memcmp(str_, x.str_, tyr_min(length_, x.length_));
    if (0 == r) {
      if (length_ < x.length_)
        r = -1;
      else if (length_ > x.length_)
        r = 1;
    }
    return r;
  }

  bool operator==(const StringPiece& r) const {
    return ((length_ == r.length_) && (0 == memcmp(str_, r.str_, length_)));
  }

  bool operator!=(const StringPiece& r) const {
    return !(operator==(r));
  }

  bool operator<(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, tyr_min(length_, r.length_));
    return ((v < 0) || ((0 == v) && (length_ < r.length_)));
  }

  bool operator<=(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, tyr_min(length_, r.length_));
    return ((v < 0) || ((0 == v) && (length_ <= r.length_)));
  }

  bool operator>(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, tyr_min(length_, r.length_));
    return ((v > 0) || ((0 == v) && (length_ > r.length_)));
  }

  bool operator>=(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, tyr_min(length_, r.length_));
    return ((v > 0) || ((0 == v) && (length_ >= r.length_)));
  }
};

std::ostream& operator<<(std::ostream& out, const StringPiece& piece);

}}

#endif // __TYR_BASIC_STRINGPIECE_HEADER_H__
