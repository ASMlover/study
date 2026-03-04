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
#include "Value.hh"

namespace ms {

class ObjString;

struct Entry {
  ObjString* key{nullptr};
  Value value{};
};

class Table {
  std::vector<Entry> entries_;
  int count_{0};

  static constexpr double kMAX_LOAD = 0.75;

  void adjust_capacity(int capacity) noexcept;
  Entry* find_entry(std::vector<Entry>& entries, ObjString* key) noexcept;
public:
  Table() noexcept = default;

  bool get(ObjString* key, Value* value) const noexcept;
  bool set(ObjString* key, Value value) noexcept;
  bool remove(ObjString* key) noexcept;
  void add_all(Table& from) noexcept;

  ObjString* find_string(cstr_t chars, sz_t length, u32_t hash) const noexcept;
  void remove_white() noexcept;
  void mark_table() noexcept;

  int count() const noexcept { return count_; }
  int capacity() const noexcept { return static_cast<int>(entries_.size()); }
  const std::vector<Entry>& entries() const noexcept { return entries_; }
};

} // namespace ms
