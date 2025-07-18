// Copyright (c) 2025 ASMlover. All rights reserved.
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

#include <string>
#include <string_view>
#include <sstream>
#include "Macros.hh"

namespace ms {

class _Safe {};
class _Unsafe {};

using nil_t                       = std::nullptr_t;
using byte_t                      = std::uint8_t;
using i8_t                        = std::int8_t;
using u8_t                        = std::uint8_t;
using i16_t                       = std::int16_t;
using u16_t                       = std::uint16_t;
using i32_t                       = std::int32_t;
using u32_t                       = std::uint32_t;
using i64_t                       = std::int64_t;
using u64_t                       = std::uint64_t;
using sz_t                        = std::size_t;
#if defined(MAPLE_GNUC)
  using ssz_t                     = ssize_t;
#else
  using ssz_t                     = std::int64_t;
#endif
using str_t                       = std::string;
using strv_t                      = std::string_view;
using cstr_t                      = const char*;
using ss_t                        = std::stringstream;
using safe_t                      = _Safe;
using unsafe_t                    = _Unsafe;

}
