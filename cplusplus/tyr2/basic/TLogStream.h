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
#ifndef __TYR_LOG_STREAM_HEADER_H__
#define __TYR_LOG_STREAM_HEADER_H__

#include "TTypes.h"
#include "TStringPiece.h"

namespace tyr {

const int kSmallBuffer = 4096;
const int kLargeBuffer = 4096 * 1000;

template <int N>
class FixedBuffer : private UnCopyable {
  typedef void (*CookieCallback)(void);

  char data_[N];
  char* current_;
  CookieCallback cookie_;
public:
  FixedBuffer(void)
    : current_(data_) {
    set_cookie(cookie_start);
  }

  ~FixedBuffer(void) {
    set_cookie(cookie_end);
  }

  const char* data(void) const {
    return data_;
  }

  size_t length(void) const {
    return static_cast<size_t>(current_ - data_);
  }

  char* current(void) {
    return current_;
  }

  size_t avail(void) const {
    return static_cast<size_t>(tail() - current_);
  }

  void remove_prefix(size_t len) {
    current_ += len;
  }

  void append(const char* buf, size_t len) {
    if (avail() > len) {
      memcpy(current_, buf, len);
      current_ += len;
    }
  }

  void reset(void) {
    current_ = data_;
  }

  void bzero(void) {
    memset(data_, 0, sizeof(data_));
  }

  void set_cookie(CookieCallback cookie) {
    cookie_ = cookie;
  }

  const char* debug_string(void);

  std::string to_string(void) const {
    return std::string(data_, length());
  }

  StringPiece to_string_piece(void) const {
    return StringPiece(data_, length());
  }
private:
  const char* tail(void) const {
    return data_ + sizeof(data_);
  }

  static void cookie_start(void);
  static void cookie_end(void);
};

class LogStream : private UnCopyable {
public:
  typedef FixedBuffer<kSmallBuffer> Buffer;
private:
  Buffer buffer_;
  static const int kMaxNumericSize = 32;
private:
  void static_check(void);
  template <typename T>
  void format_integer(T v);
public:
  LogStream& operator<<(bool v);
  LogStream& operator<<(short v);
  LogStream& operator<<(unsigned short v);
  LogStream& operator<<(int v);
  LogStream& operator<<(unsigned int v);
  LogStream& operator<<(long v);
  LogStream& operator<<(unsigned long v);
  LogStream& operator<<(long long v);
  LogStream& operator<<(unsigned long long v);
  LogStream& operator<<(const void* p);
  LogStream& operator<<(float v);
  LogStream& operator<<(double v);
  LogStream& operator<<(char v);
  LogStream& operator<<(const char* s);
  LogStream& operator<<(const byte_t* s);
  LogStream& operator<<(const std::string& s);
  LogStream& operator<<(const StringPiece& piece);
  LogStream& operator<<(const Buffer& buf);

  void append(const char* s, size_t n) {
    buffer_.append(s, n);
  }

  void reset_buffer(void) {
    buffer_.reset();
  }

  const Buffer& buffer(void) const {
    return buffer_;
  }
};

class Format {
  char data_[32];
  size_t length_;
public:
  template <typename T>
  Format(const char* fmt, T v);

  const char* data(void) const {
    return data_;
  }

  size_t length(void) const {
    return length_;
  }
};

inline LogStream& operator<<(LogStream& s, const Format& fmt) {
  s.append(fmt.data(), fmt.length());
  return s;
}

}

#endif // __TYR_LOG_STREAM_HEADER_H__
