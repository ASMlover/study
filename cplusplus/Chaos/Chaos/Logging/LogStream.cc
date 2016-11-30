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
#include <stdio.h>
#include <algorithm>
#include <limits>
#include <Chaos/Logging/LogStream.h>

namespace Chaos {

const char kDigits[] = "9876543210123456789";
const char* kZero = kDigits + 9;
static_assert(sizeof(kDigits) == 20, "wrong number of digits");

const char kDigitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(kDigitsHex) == 17, "wrong hex number of digits");

template <typename T>
size_t convert(char buf[], T value) {
  T i = value;
  char* p = buf;

  do {
    int n = static_cast<int>(i % 10);
    i /= 10;
    *p++ = kZero[n];
  } while (0 != i);

  if (value < 0)
    *p++ = '-';
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

size_t convert_hex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char* p = buf;

  do {
    int n = static_cast<int>(i % 16);
    i /= 16;
    *p++ = kDigitsHex[n];
  } while (0 != i);
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <int N> void FixedBuffer<N>::cookie_start(void) {
}

template <int N> void FixedBuffer<N>::cookie_end(void) {
}

template <int N>
const char* FixedBuffer<N>::debug_string(void) {
  return (current_ = '\0', data_);
}

void LogStream::static_check(void) {
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,
      "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,
      "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,
      "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,
      "kMaxNumericSize is large enough");
}

template <typename T>
void LogStream::format_integer(T value) {
  if (buff_.get_avail() >= kMaxNumericSize)
    buff_.remove_prefix(convert(buff_.get_current(), value));
}

LogStream& LogStream::operator<<(bool v) {
  buff_.append(v ? "1" : "0", 1);
  return *this;
}

LogStream& LogStream::operator<<(char c) {
  buff_.append(&c, 1);
  return *this;
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(float v) {
  *this << static_cast<double>(v);
  return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buff_.get_avail() >= kMaxNumericSize) {
    size_t n = snprintf(buff_.get_current(), kMaxNumericSize, "%.12g", v);
    buff_.remove_prefix(n);
  }
  return *this;
}

LogStream& LogStream::operator<<(const void* p) {
  uintptr_t value = reinterpret_cast<uintptr_t>(p);
  if (buff_.get_avail() >= kMaxNumericSize) {
    char* buf = buff_.get_current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t n = convert_hex(buf + 2, value);
    buff_.remove_prefix(n + 2);
  }
  return *this;
}

LogStream& LogStream::operator<<(const char* s) {
  if (nullptr != s)
    buff_.append(s, strlen(s));
  else
    buff_.append("(null)", 6);
  return *this;
}

LogStream& LogStream::operator<<(const byte_t* s) {
  return operator<<(reinterpret_cast<const char*>(s));
}

LogStream& LogStream::operator<<(const std::string& s) {
  buff_.append(s.data(), s.size());
  return *this;
}

LogStream& LogStream::operator<<(const StringPiece& piece) {
  buff_.append(piece.data(), piece.size());
  return *this;
}

LogStream& LogStream::operator<<(const buffer_type& buf) {
  *this << buf.to_string_piece();
  return *this;
}

template <typename T>
Format::Format(const char* fmt, T value) {
  static_assert(std::is_arithmetic<T>::value == true, "must be arithmetic type");
  size_ = snprintf(data_, sizeof(data_), fmt, value);
  CHAOS_CHECK(size_ < sizeof(data_), "`data_` space must be enough");
}

template Format::Format(const char* fmt, bool b);
template Format::Format(const char* fmt, char c);
template Format::Format(const char* fmt, short v);
template Format::Format(const char* fmt, unsigned short v);
template Format::Format(const char* fmt, int v);
template Format::Format(const char* fmt, unsigned int v);
template Format::Format(const char* fmt, long v);
template Format::Format(const char* fmt, unsigned long v);
template Format::Format(const char* fmt, long long v);
template Format::Format(const char* fmt, unsigned long long v);
template Format::Format(const char* fmt, float v);
template Format::Format(const char* fmt, double v);

}
