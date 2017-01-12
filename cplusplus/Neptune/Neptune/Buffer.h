// Copyright (c) 2017 ASMlover. All rights reserved.
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
#ifndef NEPTUNE_BUFFER_H
#define NEPTUNE_BUFFER_H

#include <cstring>
#include <algorithm>
#include <vector>
#include <Chaos/Types.h>
#include <Chaos/Container/StringPiece.h>
#include <Neptune/Endian.h>

namespace Neptune {

class Buffer {
  std::vector<char> buff_;
  std::size_t rindex_; // reader index
  std::size_t windex_; // writer index

  static const char* kCRLF;
public:
  static const std::size_t kCheapPrepend = 8;
  static const std::size_t kInitialBytes = 1024;

  explicit Buffer(std::size_t init_bytes = kInitialBytes)
    : buff_(kCheapPrepend + init_bytes)
    , rindex_(kCheapPrepend)
    , windex_(kCheapPrepend) {
  }

  ~Buffer(void) {
  }

  void swap(Buffer& r) {
    buff_.swap(r.buff_);
    std::swap(rindex_, r.rindex_);
    std::swap(windex_, r.windex_);
  }

  std::size_t get_prependable_bytes(void) const {
    return rindex_;
  }

  std::size_t get_readable_bytes(void) const {
    return windex_ - rindex_;
  }

  std::size_t get_writable_bytes(void) const {
    return buff_.size() - rindex_;
  }

  const char* peek(void) const {
    return begin() + rindex_;
  }

  std::int8_t peek_int8(void) const {
    return static_cast<std::int8_t>(*peek());
  }

  std::int16_t peek_int16(void) const {
    std::int16_t n16 = 0;
    std::memcpy(&n16, peek(), sizeof(n16));
    return Neptune::n2h16(n16);
  }

  std::int32_t peek_int32(void) const {
    std::int32_t n32 = 0;
    std::memcpy(&n32, peek(), sizeof(n32));
    return Neptune::n2h32(n32);
  }

  std::int64_t peek_int64(void) const {
    std::int64_t n64;
    std::memcpy(&n64, peek(), sizeof(n64));
    return Neptune::n2h64(n64);
  }

  char* begin_write(void) {
    return begin() + windex_;
  }

  const char* begin_write(void) const {
    return begin() + windex_;
  }

  void increment_suffix(std::size_t n) {
    windex_ += n;
  }

  void decrement_suffix(std::size_t n) {
    windex_ -= n;
  }
private:
  char* begin(void) {
    return &*buff_.begin();
  }

  const char* begin(void) const {
    return &*buff_.begin();
  }
};

}

#endif // NEPTUNE_BUFFER_H
