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
class DoubleList final : private UnCopyable {
  template <typename _Tp> struct ListNode {
    ListNode<_Tp>* prev;
    ListNode<_Tp>* next;
    _Tp value;
  };
  using NodeType = ListNode<T>;

  sz_t size_{};
  NodeType* head_{};

  inline void initialize_head() noexcept {
    head_ = SimpleAlloc<NodeType>::allocate();
    head_->prev = head_;
    head_->next = head_;
  }

  inline void destroy_head() noexcept {
    SimpleAlloc<NodeType>::deallocate(head_);
  }

  NodeType* create_aux(const T& value) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, value);
      node->prev = node->prev = nullptr;
    }
    catch (...) {
    }
    return node;
  }

  NodeType* create_aux(T&& value) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, std::move(value));
      node->prev = node->prev = nullptr;
    }
    catch (...) {
    }
    return node;
  }

  template <typename... Args> NodeType* create_aux(Args&&... args) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, std::forward<Args>(args)...);
      node->prev = node->prev = nullptr;
    }
    catch (...) {
    }
    return node;
  }

  void destroy_aux(NodeType* node) {
    destroy(&node->value);
    SimpleAlloc<NodeType>::deallocate(node);
  }

  inline void insert_aux(NodeType* pos, NodeType* new_node) noexcept {
    new_node->next = pos;
    new_node->prev = pos->prev;
    pos->prev->next = new_node;
    pos->prev = new_node;
    ++size_;
  }
public:
  DoubleList() noexcept { initialize_head(); }
  ~DoubleList() noexcept {
    clear();
    destroy_head();
  }

  inline bool empty() const noexcept { return head_->next == head_; }
  inline sz_t size() const noexcept { return size_; }
  inline T& get_head() noexcept { return head_->next->value; }
  inline const T& get_head() const noexcept { return head_->next->value; }
  inline T& get_tail() noexcept { return head_->prev->value; }
  inline const T& get_tail() const noexcept { return head_->prev->value; }

  void clear() {
    NodeType* cur = head_->next;
    while (cur != head_) {
      NodeType* node = cur;
      cur = cur->next;
      destroy(&node->value);
      SimpleAlloc<NodeType>::deallocate(node);
    }
    head_->prev = head_->next = head_;
    size_ = 0;
  }

  void append(const T& x) { insert_aux(head_, create_aux(x)); }
  void append(T&& x) { insert_aux(head_, create_aux(std::move(x))); }

  template <typename... Args> void append(Args&&... args) {
    insert_aux(head_, create_aux(std::forward<Args>(args)...));
  }

  T pop_head();
  T pop_tail();

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto* n = head_->next; n != head_; n = n->next)
      fn(n->value);
  }
};

}
