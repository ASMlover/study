// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <optional>
#include "Object.hh"

namespace ms {

// Binary format (.msc):
//   Header:  "MSC\0" (4B) + version_major (1B) + version_minor (1B) + reserved (2B)
//   u32: function_count
//   Functions[0..N-1]:  serialized in DFS post-order (inner first, top-level last)
//     u32: arity
//     u32: upvalue_count
//     u32: name_length       (0 for script-level)
//     [name_length bytes]
//     u32: script_path_length
//     [script_path_length bytes]
//     Chunk:
//       u32: code_size
//       [code_size bytes]
//       u32: constant_count
//       For each constant:
//         u8: tag (0=nil, 1=bool, 2=number, 3=string, 4=function)
//         tag-specific payload
//       u32: line_run_count
//       For each SourceRun: 4x i32 (line, column, token_length, count)

inline constexpr u8_t kMSC_VERSION_MAJOR = 1;
inline constexpr u8_t kMSC_VERSION_MINOR = 0;

bool serialize(ObjFunction* function, strv_t path) noexcept;
ObjFunction* deserialize(strv_t path) noexcept;

inline bool is_msc_file(strv_t path) noexcept {
  return path.size() >= 4 && path.substr(path.size() - 4) == ".msc";
}

} // namespace ms
