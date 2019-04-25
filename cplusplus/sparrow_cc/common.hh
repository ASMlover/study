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

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef SPARROW_DEBUG
# define CHECK(cond, msg) do {\
    if (!(cond)) {\
      std::cerr\
        << "[" << __FILE__ << ":" << __LINE__ << "] "\
        << "CHECKING FAILED: " << __func__ << "() - " << (msg) << std::endl;\
      std::abort();\
    }\
  } while (false)
#else
# define CHECK(cond, msg) ((void)0)
#endif

#define UNREACHED() do {\
  std::cerr\
    << "[" << __FILE__ << ":" << __LINE__ << "] "\
    << "UNREACHED: " << __func__ << "()" << std::endl;\
  while (true) {}\
} while (false)

namespace sparrow {

using nil_t   = std::nullptr_t;
using byte_t  = std::uint8_t;
using ssz_t   = std::int32_t;
using sz_t    = std::size_t;
using i8_t    = std::int8_t;
using u8_t    = std::uint8_t;
using i16_t   = std::int16_t;
using u16_t   = std::uint16_t;
using i32_t   = std::int32_t;
using u32_t   = std::uint32_t;
using i64_t   = std::int64_t;
using u64_t   = std::uint64_t;
using str_t   = std::string;

class Copyable {
protected:
  Copyable(void) noexcept = default;
  ~Copyable(void) noexcept = default;
};

class UnCopyable {
  UnCopyable(const UnCopyable&) = delete;
  UnCopyable& operator=(const UnCopyable&) = delete;
};

namespace Xt {
  // Xtra utilities
  template <typename I, typename E>
  inline I as_int(E x) { return static_cast<I>(x); }

  template <typename E, typename I>
  inline E as_enum(I x) { return static_cast<E>(x); }

  template <typename T>
  inline u8_t as_u8(T x) { return static_cast<u8_t>(x); }

  template <typename T> inline void* as_address(T* x) {
    return reinterpret_cast<void*>(x);
  }

  template <typename T> inline const void* as_address(const T* x) {
    return reinterpret_cast<const void*>(x);
  }

  template <typename T> inline byte_t* as_bytes(T* x) {
    return reinterpret_cast<byte_t*>(x);
  }

  template <typename T> inline const byte_t* as_bytes(const T* x) {
    return reinterpret_cast<const byte_t*>(x);
  }

  template <typename Target, typename Source> inline Target* cast(Source* x) {
    return static_cast<Target*>(x);
  }

  template <typename Target, typename Source> inline Target* down(Source* x) {
    return dynamic_cast<Target*>(x);
  }
}

enum class ErrType {
  IO,
  MEMORY,
  LEXICAL,
  COMPILE,
  RUNTIME,
};

void error_report(ErrType type,
    const char* fname, int line, const char* func, const char* format, ...);

}

#define IO_ERR(fmt, ...) sparrow::error_report(\
    sparrow::ErrType::IO, __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)
#define MEM_ERR(fmt, ...) sparrow::error_report(\
    sparrow::ErrType::MEMORY, __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)
#define LEX_ERR(fmt, ...) sparrow::error_report(\
    sparrow::ErrType::LEXICAL, __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)
#define COMPILE_ERR(fmt, ...) sparrow::error_report(\
    sparrow::ErrType::COMPILE, __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)
#define RUNTIME_ERR(fmt, ...) sparrow::error_report(\
    sparrow::ErrType::RUNTIME, __FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)
