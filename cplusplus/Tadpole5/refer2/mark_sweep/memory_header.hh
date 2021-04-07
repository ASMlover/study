// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___           ____    ____
// /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
//                             \ \_\
//                              \/_/
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

#include <ostream>
#include "common.hh"

namespace tadpole::gc {

enum class MemoryTag : u8_t {
  MEMORY    = 0x00,
  UNMARKED  = 0x01,
  MARKED    = 0x02,
};

inline std::ostream& operator<<(std::ostream& out, MemoryTag tag) noexcept {
  return out << tadpole::gc::as_type<int>(tag);
}

inline MemoryTag operator|(MemoryTag a, MemoryTag b) noexcept {
  return as_type<MemoryTag>(as_type<u8_t>(a) | as_type<u8_t>(b));
}

inline MemoryTag operator|=(MemoryTag& a, MemoryTag b) noexcept {
  return a = a | b, a;
}

inline MemoryTag operator&(MemoryTag a, MemoryTag b) noexcept {
  return as_type<MemoryTag>(as_type<u8_t>(a) & as_type<u8_t>(b));
}

inline MemoryTag operator&=(MemoryTag& a, MemoryTag b) noexcept {
  return a = a & b, a;
}

inline MemoryTag operator~(MemoryTag a) noexcept {
  return as_type<MemoryTag>(~as_type<u8_t>(a));
}

struct MemoryHeader {
  MemoryTag tag_{MemoryTag::MEMORY};
  u32_t size_{};
  MemoryHeader* next_{};

  MemoryHeader() noexcept {}
  MemoryHeader(MemoryTag tag) noexcept : tag_(tag) {}
  MemoryHeader(MemoryTag tag, u32_t sz = 0, MemoryHeader* next = nullptr) noexcept
    : tag_(tag), size_(sz), next_(next) {
  }

  inline MemoryTag tag() const noexcept { return tag_; }
  inline void set_tag(MemoryTag tag) noexcept { tag_ = tag; }
  inline bool is_memory() const noexcept { return tag_ == MemoryTag::MEMORY; }
  inline bool is_object() const noexcept { return tag_ != MemoryTag::MEMORY; }
  inline void set_as_memory() noexcept { tag_ = MemoryTag::MEMORY; }
  inline void set_as_object() noexcept { tag_ = MemoryTag::UNMARKED; }
  inline bool is_marked() const noexcept { return as_type<bool>(tag_ & MemoryTag::MARKED); }
  inline void set_marked() noexcept { tag_ |= MemoryTag::MARKED; }
  inline void unset_marked() noexcept { tag_ &= ~MemoryTag::MARKED; }
  inline u32_t size() const noexcept { return size_; }
  template <typename N> inline void set_size(N n) noexcept { size_ = as_type<u32_t>(n); }
  inline MemoryHeader* next() const noexcept { return next_; }
  inline void set_next(MemoryHeader* next = nullptr) noexcept { next_ = next; }
};

static constexpr u32_t kHeaderSize = sizeof(MemoryHeader);

template <typename Object> inline sz_t get_objsize() noexcept { return kHeaderSize + sizeof(Object); }

template <typename T, typename U> inline T* as_ptr(U* p) noexcept { return (T*)p; }

template <typename Object> inline MemoryHeader* as_memory(Object* p) noexcept {
  return as_ptr<MemoryHeader>(p) - 1;
}

template <typename Object> inline Object* as_object(MemoryHeader* p) noexcept {
  return as_ptr<Object>(p + 1);
}

}
