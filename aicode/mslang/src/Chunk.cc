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

void Chunk::write(u8_t byte, int line, int column, int token_length) noexcept {
  code_.push_back(byte);
  if (!lines_.empty() && lines_.back().line == line
      && lines_.back().column == column
      && lines_.back().token_length == token_length) {
    lines_.back().count++;
  } else {
    lines_.push_back({line, column, token_length, 1});
  }
}

void Chunk::write(OpCode op, int line, int column, int token_length) noexcept {
  write(static_cast<u8_t>(op), line, column, token_length);
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
  sz_t cumulative = 0;
  for (const auto& run : lines_) {
    cumulative += static_cast<sz_t>(run.count);
    if (offset < cumulative) {
      return run.line;
    }
  }
  return 0;
}

int Chunk::column_at(sz_t offset) const noexcept {
  sz_t cumulative = 0;
  for (const auto& run : lines_) {
    cumulative += static_cast<sz_t>(run.count);
    if (offset < cumulative) {
      return run.column;
    }
  }
  return 0;
}

int Chunk::token_length_at(sz_t offset) const noexcept {
  sz_t cumulative = 0;
  for (const auto& run : lines_) {
    cumulative += static_cast<sz_t>(run.count);
    if (offset < cumulative) {
      return run.token_length;
    }
  }
  return 0;
}

sz_t Chunk::count() const noexcept {
  return code_.size();
}

void Chunk::truncate(sz_t new_count) noexcept {
  // Shrink the line-run table to match the new code size
  sz_t to_remove = code_.size() - new_count;
  while (to_remove > 0 && !lines_.empty()) {
    auto& back = lines_.back();
    sz_t take = std::min(to_remove, static_cast<sz_t>(back.count));
    back.count -= static_cast<int>(take);
    to_remove -= take;
    if (back.count == 0) lines_.pop_back();
  }
  code_.resize(new_count);
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
