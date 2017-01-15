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

class Buffer : public Chaos::Copyable {
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

  std::size_t prependable_bytes(void) const {
    return rindex_;
  }

  std::size_t readable_bytes(void) const {
    return windex_ - rindex_;
  }

  std::size_t writable_bytes(void) const {
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

  void forward_written(std::size_t n) {
    windex_ += n;
  }

  void backward_written(std::size_t n) {
    windex_ -= n;
  }

  const char* find_crlf(void) const {
    const char* crlf = std::search(peek(), begin_write(), kCRLF, kCRLF + 2);
    return crlf == begin_write() ? nullptr : crlf;
  }

  const char* find_crlf(const char* start) const {
    const char* crlf = std::search(start, begin_write(), kCRLF, kCRLF + 2);
    return crlf == begin_write() ? nullptr : crlf;
  }

  const char* find_eol(void) const {
    const void* eol = std::memchr(peek(), '\n', readable_bytes());
    return static_cast<const char*>(eol);
  }

  const char* find_eol(const char* start) const {
    const void* eol = std::memchr(start, '\n', begin_write() - start);
    return static_cast<const char*>(eol);
  }

  void retrieve(std::size_t n) {
    if (n < readable_bytes())
      rindex_ += n;
    else
      retrieve_all();
  }

  void retrieve_all(void) {
    rindex_ = kCheapPrepend;
    windex_ = kCheapPrepend;
  }

  void retrieve_until(const char* end) {
    retrieve(end - peek());
  }

  void retrieve_int8(void) {
    retrieve(sizeof(std::int8_t));
  }

  void retrieve_int16(void) {
    retrieve(sizeof(std::int16_t));
  }

  void retrieve_int32(void) {
    retrieve(sizeof(std::int32_t));
  }

  void retrieve_int64(void) {
    retrieve(sizeof(std::int64_t));
  }

  std::string retrieve_to_string(std::size_t n) {
    std::string result(peek(), n);
    retrieve(n);
    return result;
  }

  std::string retrieve_all_to_string(void) {
    return retrieve_to_string(readable_bytes());
  }

  Chaos::StringPiece to_string_piece(void) const {
    return Chaos::StringPiece(peek(), readable_bytes());
  }

  void ensure_writable_bytes(std::size_t n) {
    if (writable_bytes() < n)
      resize(n);
    CHAOS_CHECK(writable_bytes() >= n, "Neptune::Buffer - writable bytes not enough");
  }

  void prepend(const void* buf, std::size_t len) {
    rindex_ -= len;
    const char* s = static_cast<const char*>(buf);
    std::copy(s, s + len, begin() + rindex_);
  }

  void prepend_int8(std::int8_t i8) {
    prepend(&i8, sizeof(i8));
  }

  void prepend_int16(std::int16_t i16) {
    std::int16_t n16 = Neptune::h2n16(i16);
    prepend(&n16, sizeof(n16));
  }

  void prepend_int32(std::int32_t i32) {
    std::int32_t n32 = Neptune::h2n32(i32);
    prepend(&n32, sizeof(n32));
  }

  void prepend_int64(std::int64_t i64) {
    std::int64_t n64 = Neptune::h2n64(i64);
    prepend(&n64, sizeof(n64));
  }

  void append(const char* buf, std::size_t len) {
    ensure_writable_bytes(len);
    std::copy(buf, buf + len, begin_write());
    forward_written(len);
  }

  void append(const void* buf, std::size_t len) {
    append(static_cast<const char*>(buf), len);
  }

  void append(const Chaos::StringPiece& piece) {
    append(piece.data(), piece.size());
  }

  void append_int8(std::int8_t i8) {
    append(&i8, sizeof(i8));
  }

  void append_int16(std::int16_t i16) {
    std::int16_t n16 = Neptune::h2n16(i16);
    append(&n16, sizeof(n16));
  }

  void append_int32(std::int32_t i32) {
    std::int32_t n32 = Neptune::h2n32(i32);
    append(&n32, sizeof(n32));
  }

  void append_int64(std::int64_t i64) {
    std::int64_t n64 = Neptune::h2n64(i64);
    append(&n64, sizeof(n64));
  }

  std::int8_t read_int8(void) {
    std::int8_t i8 = peek_int8();
    retrieve_int8();
    return i8;
  }

  std::int16_t read_int16(void) {
    std::int16_t i16 = peek_int16();
    retrieve_int16();
    return i16;
  }

  std::int32_t read_int32(void) {
    std::int32_t i32 = peek_int32();
    retrieve_int32();
    return i32;
  }

  std::int64_t read_int64(void) {
    std::int64_t i64 = peek_int64();
    retrieve_int64();
    return i64;
  }

  void shrink(std::size_t reserve) {
    Buffer other;
    other.ensure_writable_bytes(readable_bytes() + reserve);
    other.append(to_string_piece());
    swap(other);
  }

  std::size_t internal_capacity(void) const {
    return buff_.capacity();
  }

  int read_sockfd(int sockfd, int& saved_errno);
private:
  char* begin(void) {
    return &*buff_.begin();
  }

  const char* begin(void) const {
    return &*buff_.begin();
  }

  void resize(std::size_t bytes) {
    if (writable_bytes() + prependable_bytes() < bytes + kCheapPrepend) {
      buff_.resize(windex_ + bytes);
    }
    else {
      CHAOS_CHECK(kCheapPrepend < rindex_, "reader index should begin after `kCheapPrepend`");
      std::size_t readable = readable_bytes();
      std::copy(begin() + rindex_, begin() + windex_, begin() + kCheapPrepend);
      rindex_ = kCheapPrepend;
      windex_ = rindex_ + readable;
    }
  }
};

}

#endif // NEPTUNE_BUFFER_H
