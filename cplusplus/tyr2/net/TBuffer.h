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
#ifndef __TYR_NET_BUFFER_HEADER_H__
#define __TYR_NET_BUFFER_HEADER_H__

#include <assert.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include "../basic/TTypes.h"
#include "../basic/TStringPiece.h"
#include "TEndian.h"

namespace tyr { namespace net {

class Buffer {
  std::vector<char> buff_;
  size_t rindex_; // reader index
  size_t windex_; // writer index
  static const char* kCRLF = "\r\n";
public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t init_size = kInitialSize)
    : buff_(kCheapPrepend + init_size)
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

  size_t prependable_bytes(void) const {
    return rindex_;
  }

  size_t readable_bytes(void) const {
    return windex_ - rindex_;
  }

  size_t writable_bytes(void) const {
    return buff_.size() - rindex_;
  }

  const char* peek(void) const {
    return begin() + rindex_;
  }

  int8_t peek_int8(void) const {
    return static_cast<int8_t>(*peek());
  }

  int16_t peek_int16(void) const {
    int16_t be16 = 0;
    memcpy(&be16, peek(), sizeof(be16));
    return net_to_host16(be16);
  }

  int32_t peek_int32(void) const {
    int32_t be32 = 0;
    memcpy(&be32, peek(), sizeof(be32));
    return net_to_host32(be32);
  }

  int64_t peek_int64(void) const {
    int64_t be64 = 0;
    memcpy(&be64, peek(), sizeof(be64));
    return net_to_host64(be64);
  }

  char* begin_write(void) {
    return begin() + windex_;
  }

  const char* begin_write(void) const {
    return begin() + windex_;
  }

  void increment_suffix(size_t n) {
    windex_ += n;
  }

  void decrement_suffix(size_t n) {
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
    const void* eol = memchr(peek(), '\n', readable_bytes());
    return static_cast<const char*>(eol);
  }

  const char* find_eol(const char* start) const {
    const void* eol = memchr(start, '\n', begin_write() - start);
    return static_cast<const char*>(eol);
  }

  void retrieve(size_t n) {
    if (n < readable_bytes())
      rindex_ += n;
    else
      retrieve_all();
  }

  void retrieve_until(const char* end) {
    retrieve(end - peek());
  }

  void retrieve_all(void) {
    rindex_ = kCheapPrepend;
    windex_ = kCheapPrepend;
  }

  void retrieve_int8(void) {
    retrieve(sizeof(int8_t));
  }

  void retrieve_int16(void) {
    retrieve(sizeof(int16_t));
  }

  void retrieve_int32(void) {
    retrieve(sizeof(int32_t));
  }

  void retrieve_int64(void) {
    retrieve(sizeof(int64_t));
  }

  std::string retrieve_to_string(size_t n) {
    std::string result(peek(), n);
    retrieve(n);
    return result;
  }

  std::string retrieve_all_to_string(void) {
    return retrieve_as_string(readable_bytes());
  }

  tyr::basic::StringPiece to_string_piece(void) const {
    return tyr::basic::StringPiece(peek(), readable_bytes());
  }

  void ensure_writable_bytes(size_t n) {
    if (writable_bytes() < n)
      resize(n);
    assert(writable_bytes() >= n);
  }

  void prepend(const void* buf, size_t len) {
    rindex_ -= len;
    const char* s = static_cast<const char*>(buf);
    std::copy(s, s + len, begin() + rindex_);
  }

  void prepend_int8(int8_t i) {
    prepend(&i, sizeof(i));
  }

  void prepend_int16(int16_t i) {
    int16_t be16 = host_to_net16(i);
    prepend(&be16, sizeof(be16));
  }

  void prepend_int32(int32_t i) {
    int32_t be32 = host_to_net32(i);
    prepend(&be32, sizeof(be32));
  }

  void prepend_int64(int64_t i) {
    int64_t be64 = host_to_net64(i);
    prepend(&be64, sizeof(be64));
  }

  void append(const char* buf, size_t len) {
    ensure_writable_bytes(len);
    std::copy(buf, buf + len, begin_write());
    increment_suffix(len);
  }

  void append(const void* buf, size_t len) {
    append(static_cast<const char*>(buf), len);
  }

  void append(const tyr::basic::StringPiece& piece) {
    append(piece.data(), piece.size());
  }

  void append_int8(int8_t i) {
    append(&i, sizeof(i));
  }

  void append_int16(int16_t i) {
    int16_t be16 = host_to_net16(i);
    append(&be16, sizeof(be16));
  }

  void append_int32(int32_t i) {
    int32_t be32 = host_to_net32(i);
    append(&be32, sizeof(be32));
  }

  void append_int64(int64_t i) {
    int64_t be64 = host_to_net64(i);
    append(&be64, sizeof(be64));
  }

  int8_t read_int8(void) {
    int8_t value = peek_int8();
    retrieve_int8();
    return value;
  }

  int16_t read_int16(void) {
    int16_t value = peek_int16();
    retrieve_int16();
    return value;
  }

  int32_t read_int32(void) {
    int32_t value = peek_int32();
    retrieve_int32();
    return value;
  }

  int64_t read_int64(void) {
    int64_t value = peek_int64();
    retrieve_int64();
    return value;
  }

  void shrink(size_t reserve) {
    Buffer other;
    other.ensure_writable_bytes(readable_bytes() + reserve);
    other.append(to_string_piece());
    swap(other);
  }

  size_t internal_capacity(void) const {
    return buff_.capacity();
  }

  ssize_t read_fd(int fd, int& saved_errno);
private:
  char* begin(void) {
    return &*buff_.begin();
  }

  const char* begin(void) const {
    return &*buff_.begin();
  }

  void resize(size_t bytes) {
    if (writable_bytes() + prependable_bytes() < bytes + kCheapPrepend) {
      buff_.resize(windex_ + bytes);
    }
    else {
      assert(kCheapPrepend < rindex_);
      size_t readable = readable_bytes();
      std::copy(begin() + rindex_, begin() + windex_, begin() + kCheapPrepend);
      rindex_ = kCheapPrepend;
      windex_ = rindex_ + readable;
    }
  }
};

}}

#endif // __TYR_NET_BUFFER_HEADER_H__
