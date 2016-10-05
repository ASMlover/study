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
#ifndef DEVIL_UTILITY_STRINGPIECE_HEADER_H
#define DEVIL_UTILITY_STRINGPIECE_HEADER_H

#include <string>
#include "Types.h"

namespace devil { namespace utility {

class StringArg {
  const char* str_;
public:
  StringArg(const char* s)
    : str_(s) {
  }

  StringArg(const std::string& s)
    : str_(s.c_str()) {
  }

  const char* Data(void) const {
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

  const char* Data(void) const {
    return str_;
  }

  size_t Size(void) const {
    return length_;
  }

  char operator[](std::ptrdiff_t i) const {
    return str_[i];
  }

  bool Empty(void) const {
    return 0 == length_;
  }

  const char* Begin(void) const {
    return str_;
  }

  const char* End(void) const {
    return str_ + length_;
  }

  void Clear(void) {
    str_ = nullptr;
    length_ = 0;
  }

  void Set(const char* s) {
    str_ = s;
    length_ = strlen(str_);
  }

  void Set(const char* s, size_t n) {
    str_ = s;
    length_ = n;
  }

  void Set(const void* buf, size_t len) {
    str_ = reinterpret_cast<const char*>(buf);
    length_ = len;
  }

  void RemovePrefix(size_t n) {
    str_ += n;
    length_ -= n;
  }

  void RemoveSuffix(size_t n) {
    length_ -= n;
  }

  std::string ToString(void) const {
    return std::string(str_, length_);
  }

  void CopyToString(std::string* x) const {
    x->assign(str_, length_);
  }

  bool StartsWith(const StringPiece& x) const {
    return ((length_ >= x.length_) && (0 == memcmp(str_, x.str_, x.length_)));
  }

  int Compare(const StringPiece& x) const {
    int r = memcmp(str_, x.str_, std::min(length_, x.length_));
    if (0 == r) {
      if (length_ < x.length_)
        r = -1;
      else if (length_ > x.length_)
        r = 1;
    }
    return r;
  }

  bool operator<(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, std::min(length_, r.length_));
    return ((v < 0) || ((0 == v) && (length_ < r.length_)));
  }

  bool operator<=(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, std::min(length_, r.length_));
    return ((v < 0) || ((0 == v) && (length_ <= length_)));
  }

  bool operator>(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, std::min(length_, r.length_));
    return ((v > 0) || ((0 == v) && (length_ > r.length_)));
  }

  bool operator>=(const StringPiece& r) const {
    int v = memcmp(str_, r.str_, std::min(length_, r.length_));
    return ((v > 0) || ((0 == v) && (length_ >= r.length_)));
  }
};

}}

#endif // DEVIL_UTILITY_STRINGPIECE_HEADER_H
