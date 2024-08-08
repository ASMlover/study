// Copyright (c) 2024 ASMlover. All rights reserved.
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

#include <cassert>
#include <cstring>
#include "common.hh"

namespace ev {

class Slice final : private UnCopyable {
  cstr_t                data_;
  sz_t                  size_;
public:
  Slice() noexcept : data_{""}, size_{} {}
  Slice(cstr_t data, sz_t size) noexcept : data_{data}, size_{size} {}
  Slice(const str_t& s) noexcept : data_{s.data()}, size_{s.size()} {}
  Slice(cstr_t s) noexcept : data_{s}, size_{strlen(s)} {}

  inline cstr_t data() const noexcept {
    return data_;
  }

  inline sz_t size() const noexcept {
    return size_;
  }

  inline bool is_empty() const noexcept {
    return 0 == size_;
  }

  inline char operator[](sz_t i) const noexcept {
    assert(i < size_);
    return data_[i];
  }

  inline void clear() noexcept {
    data_ = "";
    size_ = 0;
  }

  inline void remove_prefix(sz_t n) noexcept {
    assert(n < size_);
    data_ += n;
    size_ -= n;
  }

  inline str_t to_string() const noexcept {
    return str_t(data_, size_);
  }

  inline bool operator==(const Slice& x) const noexcept {
    return (size_ == x.size_ && 0 == std::memcmp(data_, x.data_, x.size_));
  }

  inline bool operator!=(const Slice& x) const noexcept {
    return !(*this == x);
  }

  inline bool operator<(const Slice& x) const noexcept {
    return compare(x) < 0;
  }

  inline int compare(const Slice& x) const noexcept {
    const sz_t min_len = (size_ < x.size_) ? size_ : x.size_;
    int r = std::memcmp(data_, x.data_, min_len);

    if (0 == r) {
      if (size_ < x.size_)
        r = -1;
      else if (size_ > x.size_)
        r = 1;
    }
    return r;
  }
};

}
