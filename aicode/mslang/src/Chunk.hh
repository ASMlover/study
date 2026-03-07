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

#include <vector>
#include "Types.hh"
#include "Opcode.hh"
#include "Value.hh"

namespace ms {

class Chunk {
  struct LineRun {
    int line;
    int count;
  };

  std::vector<u8_t> code_;
  std::vector<Value> constants_;
  std::vector<LineRun> lines_;
public:
  // Write a raw byte
  void write(u8_t byte, int line) noexcept;

  // Write an opcode
  void write(OpCode op, int line) noexcept;

  // Add a constant and return its index
  sz_t add_constant(Value value) noexcept;

  // Accessors
  u8_t code_at(sz_t offset) const noexcept;
  const Value& constant_at(sz_t index) const noexcept;
  int line_at(sz_t offset) const noexcept;
  sz_t count() const noexcept;

  // For patching (jump offsets etc.)
  u8_t& operator[](sz_t offset) noexcept;

  // Access underlying data (for VM ip pointer)
  const u8_t* code_data() const noexcept;
  u8_t* code_data() noexcept;

  // Constants access
  const std::vector<Value>& constants() const noexcept;
};

} // namespace ms
