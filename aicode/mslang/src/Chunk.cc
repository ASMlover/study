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
#include "Chunk.hh"

namespace ms {

void Chunk::write(u8_t byte, int line) noexcept {
  code_.push_back(byte);
  lines_.push_back(line);
}

void Chunk::write(OpCode op, int line) noexcept {
  write(static_cast<u8_t>(op), line);
}

sz_t Chunk::add_constant(Value value) noexcept {
  constants_.push_back(value);
  return constants_.size() - 1;
}

u8_t Chunk::code_at(sz_t offset) const noexcept {
  return code_[offset];
}

const Value& Chunk::constant_at(sz_t index) const noexcept {
  return constants_[index];
}

int Chunk::line_at(sz_t offset) const noexcept {
  return lines_[offset];
}

sz_t Chunk::count() const noexcept {
  return code_.size();
}

u8_t& Chunk::operator[](sz_t offset) noexcept {
  return code_[offset];
}

const u8_t* Chunk::code_data() const noexcept {
  return code_.data();
}

u8_t* Chunk::code_data() noexcept {
  return code_.data();
}

const std::vector<Value>& Chunk::constants() const noexcept {
  return constants_;
}

} // namespace ms
