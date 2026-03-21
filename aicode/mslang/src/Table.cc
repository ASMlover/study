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
#include <cstring>
#include "Object.hh"
#include "Table.hh"

namespace ms {

void mark_object(Object* object) noexcept;
void mark_value(Value& value) noexcept;

Entry* Table::find_entry(std::vector<Entry>& entries, sz_t mask, ObjString* key) noexcept {
  sz_t index = key->hash() & mask;

  for (;;) {
    Entry* entry = &entries[index];
    if (entry->key == nullptr) return entry;
    if (entry->key == key) return entry;

    index = (index + 1) & mask;
  }
}

const Entry* Table::find_entry(const std::vector<Entry>& entries, sz_t mask, ObjString* key) const noexcept {
  sz_t index = key->hash() & mask;

  for (;;) {
    const Entry* entry = &entries[index];
    if (entry->key == nullptr) return entry;
    if (entry->key == key) return entry;

    index = (index + 1) & mask;
  }
}

void Table::adjust_capacity(int capacity) noexcept {
  std::vector<Entry> new_entries(static_cast<sz_t>(capacity));
  sz_t new_mask = static_cast<sz_t>(capacity) - 1;

  count_ = 0;
  for (auto& entry : entries_) {
    if (entry.key == nullptr) continue;

    Entry* dest = find_entry(new_entries, new_mask, entry.key);
    dest->key = entry.key;
    dest->value = entry.value;
    count_++;
  }

  entries_ = std::move(new_entries);
  mask_ = new_mask;
}

bool Table::get(ObjString* key, Value* value) const noexcept {
  if (count_ == 0) return false;

  const Entry* entry = find_entry(entries_, mask_, key);
  if (entry->key == nullptr) return false;

  *value = entry->value;
  return true;
}

bool Table::set(ObjString* key, Value value) noexcept {
  if (count_ + 1 > static_cast<int>(entries_.size() * kMAX_LOAD)) {
    int cap = entries_.empty() ? 8 : static_cast<int>(entries_.size()) * 2;
    adjust_capacity(cap);
  }

  Entry* entry = find_entry(entries_, mask_, key);
  bool is_new = (entry->key == nullptr);
  if (is_new) count_++;

  entry->key = key;
  entry->value = value;
  return is_new;
}

bool Table::remove(ObjString* key) noexcept {
  if (count_ == 0) return false;

  Entry* entry = find_entry(entries_, mask_, key);
  if (entry->key == nullptr) return false;

  sz_t idx = static_cast<sz_t>(entry - entries_.data());
  entries_[idx] = Entry{};
  count_--;

  // Backward-shift: move displaced entries back toward their natural slot
  sz_t empty = idx;
  for (sz_t i = (empty + 1) & mask_;
       entries_[i].key != nullptr;
       i = (i + 1) & mask_) {
    sz_t natural = entries_[i].key->hash() & mask_;
    bool displaced = (empty < i)
        ? (natural <= empty || natural > i)
        : (natural <= empty && natural > i);
    if (displaced) {
      entries_[empty] = entries_[i];
      entries_[i] = Entry{};
      empty = i;
    }
  }
  return true;
}

void Table::add_all(Table& from) noexcept {
  for (auto& entry : from.entries_) {
    if (entry.key != nullptr) {
      set(entry.key, entry.value);
    }
  }
}

ObjString* Table::find_string(cstr_t chars, sz_t length, u32_t hash) const noexcept {
  if (count_ == 0) return nullptr;

  sz_t index = hash & mask_;

  for (;;) {
    const Entry& entry = entries_[index];
    if (entry.key == nullptr) return nullptr;
    if (entry.key->hash() == hash &&
               entry.key->value().length() == length &&
               std::memcmp(entry.key->value().c_str(), chars, length) == 0) {
      return entry.key;
    }

    index = (index + 1) & mask_;
  }
}

void Table::remove_white() noexcept {
  std::vector<ObjString*> to_remove;
  for (auto& entry : entries_) {
    if (entry.key != nullptr && !entry.key->is_marked())
      to_remove.push_back(entry.key);
  }
  for (auto* key : to_remove) remove(key);
}

void Table::mark_table() noexcept {
  for (auto& entry : entries_) {
    mark_object(entry.key);
    mark_value(entry.value);
  }
}

} // namespace ms
