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

#include "Object.hh"

namespace ms {

// Slab pool for fixed-size objects; 64 objects per slab.
// alloc() returns raw memory; caller must use placement new.
// free()  expects the destructor to have already been called.
template <typename T>
class ObjectPool {
  static constexpr sz_t kSlabSize = 64;
  struct Slab { alignas(T) char data[sizeof(T) * kSlabSize]; };
  struct FreeNode { FreeNode* next; };

  std::vector<std::unique_ptr<Slab>> slabs_;
  FreeNode*  free_list_{nullptr};
  sz_t       allocated_{0};

public:
  void* alloc() noexcept {
    if (free_list_ == nullptr) {
      auto slab = std::make_unique<Slab>();
      char* base = slab->data;
      for (sz_t i = 0; i < kSlabSize; ++i) {
        auto* node = reinterpret_cast<FreeNode*>(base + i * sizeof(T));
        node->next = free_list_;
        free_list_ = node;
      }
      slabs_.push_back(std::move(slab));
    }
    FreeNode* node = free_list_;
    free_list_ = node->next;
    ++allocated_;
    return static_cast<void*>(node);
  }

  void free(T* obj) noexcept {
    auto* node = reinterpret_cast<FreeNode*>(obj);
    node->next = free_list_;
    free_list_ = node;
    --allocated_;
  }

  void destroy_all() noexcept {
    slabs_.clear();
    free_list_ = nullptr;
    allocated_ = 0;
  }

  sz_t size_bytes() const noexcept { return slabs_.size() * sizeof(Slab); }
};

void mark_object(Object* object) noexcept;
void mark_value(Value& value) noexcept;

// Write barrier: call when an old-gen object stores a reference to any object.
// If the referenced object is young, the old object is added to the remembered set.
void write_barrier(Object* owner, Object* ref) noexcept;
void write_barrier_value(Object* owner, Value& value) noexcept;

} // namespace ms
