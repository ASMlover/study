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

struct SourceRun {
  int line;
  int column;
  int token_length;
  int count;
};

class Chunk {
  std::vector<Instruction> code_;
  std::vector<Value> constants_;
  std::vector<SourceRun> lines_;
public:
  // Write a 32-bit instruction
  void write(Instruction instr, int line, int column = 0, int token_length = 0) noexcept;

  // Add a constant and return its index
  sz_t add_constant(Value value) noexcept;

  // Accessors
  Instruction code_at(sz_t offset) const noexcept;
  const Value& constant_at(sz_t index) const noexcept;
  int line_at(sz_t offset) const noexcept;
  int column_at(sz_t offset) const noexcept;
  int token_length_at(sz_t offset) const noexcept;
  sz_t count() const noexcept;

  // For patching (jump offsets, IC slots, etc.)
  Instruction& operator[](sz_t offset) noexcept;

  // Truncate bytecode to given size (for constant folding)
  void truncate(sz_t new_count) noexcept;

  // Access underlying data (for VM instruction pointer)
  const Instruction* code_data() const noexcept;
  Instruction* code_data() noexcept;

  // Constants access
  const std::vector<Value>& constants() const noexcept;
  std::vector<Value>& constants() noexcept;

  // Lines access (for serialization)
  const std::vector<SourceRun>& lines() const noexcept;
  std::vector<SourceRun>& lines() noexcept;

  // Direct code access (for serialization / deserialization)
  std::vector<Instruction>& code() noexcept;
  const std::vector<Instruction>& code() const noexcept;
};

} // namespace ms
