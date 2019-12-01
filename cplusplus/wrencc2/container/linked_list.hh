// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include "helper.hh"

namespace wrencc {

template <typename T>
class LinkedList final : private UnCopyable {
  template <typename _Tp> struct ListNode {
    ListNode<_Tp>* next;
    _Tp value;
  };
  using NodeType = ListNode<T>;

  sz_t size_{};
  NodeType* head_{};
  NodeType* tail_{};

  NodeType* create_aux(const T& value) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, value);
      node->next = nullptr;
    }
    catch (...) {
      SimpleAlloc<NodeType>::deallocate(node);
      throw;
    }
    return node;
  }

  NodeType* create_aux(T&& value) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, std::move(value));
      node->next = nullptr;
    }
    catch (...) {
      SimpleAlloc<NodeType>::deallocate(node);
      throw;
    }
    return node;
  }

  template <typename... Args> NodeType* create_aux(Args&&... args) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, std::forward<Args>(args)...);
      node->next = nullptr;
    }
    catch (...) {
      SimpleAlloc<NodeType>::deallocate(node);
      throw;
    }
    return node;
  }

  void destroy_aux(NodeType* node) noexcept {
    destroy(&node->value);
    SimpleAlloc<NodeType>::deallocate(node);
  }

  void append_aux(NodeType* new_node) {
    if (head_ == nullptr) {
      head_ = tail_ = new_node;
    }
    else {
      tail_->next = new_node;
      tail_ = new_node;
    }
    ++size_;
  }
public:
  LinkedList() noexcept {}
  ~LinkedList() noexcept { clear(); }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline T& get_head() noexcept { return head_->value; }
  inline const T& get_head() const noexcept { return head_->value; }
  inline T& get_tail() noexcept { return tail_->value; }
  inline const T& get_tail() const noexcept { return tail_->value; }

  void clear() {
    while (head_ != nullptr) {
      auto* node = head_;
      head_ = head_->next;
      destroy_aux(node);
    }
    tail_ = nullptr;
    size_ = 0;
  }

  inline void append(const T& x) { append_aux(create_aux(x)); }
  inline void append(T&& x) { append_aux(create_aux(std::move(x))); }

  template <typename... Args> inline void append(Args&&... args) {
    append_aux(create_aux(std::forward<Args>(args)...));
  }

  T pop_head() {
    NodeType* node = head_;
    if (head_ = head_->next; head_ == nullptr)
      tail_ = nullptr;
    T r = node->value;
    destroy_aux(node);
    --size_;

    return r;
  }

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto* n = head_; n != nullptr; n = n->next)
      fn(n->value);
  }
};

}
