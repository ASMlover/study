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

namespace nyx {

using byte_t = unsigned char;
using sz_t = std::size_t;
using i8_t = std::int8_t;
using u8_t = std::uint8_t;
using i16_t = std::int16_t;
using u16_t = std::uint16_t;
using i32_t = std::int32_t;
using u32_t = std::uint32_t;
using i64_t = std::int64_t;
using u64_t = std::uint64_t;

class Copyable {
protected:
  Copyable(void) = default;
  ~Copyable(void) = default;
};

class UnCopyable {
  UnCopyable(const UnCopyable&) = delete;
  UnCopyable& operator=(const UnCopyable&) = delete;
protected:
  UnCopyable(void) = default;
  ~UnCopyable(void) = default;
};

namespace Xenum {
  template <typename Int, typename Enum>
  inline Int as_int(Enum x) { return static_cast<Int>(x); }

  template <typename Enum, typename Int>
  inline Enum as_enum(Int x) { return static_cast<Enum>(x); }
}

namespace Xptr {
  template <typename Class>
  inline void* address(Class* x) { return reinterpret_cast<void*>(x); }

  template <typename Class>
  inline const void* address(const Class* x) {
    return reinterpret_cast<const void*>(x);
  }

  template <typename Source>
  inline byte_t* bytes(Source* x) { return reinterpret_cast<byte_t*>(x); }

  template <typename Source>
  inline const byte_t* bytes(const Source* x) {
    return reinterpret_cast<const byte_t*>(x);
  }

  template <typename Target, typename Source>
  inline Target* cast(Source* x) { return static_cast<Target*>(x); }

  template <typename Target, typename Source>
  inline Target* down(Source* x) { return dynamic_cast<Target*>(x); }
}

}
