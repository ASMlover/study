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

template <typename Tp> struct LinkedListNode {
  LinkedListNode<Tp>* next;
  Tp value;
};

template <typename _Tp, typename _Ref, typename _Ptr> struct LinkedListIter {
  using Iter      = LinkedListIter<_Tp, _Tp&, _Tp*>;
  using Self      = LinkedListIter<_Tp, _Ref, _Ptr>;
  using ValueType = _Tp;
  using Ref       = _Ref;
  using Ptr       = _Ptr;
  using NodeType  = LinkedListNode<_Tp>;

  NodeType* node{};

  LinkedListIter() noexcept {}
  LinkedListIter(NodeType* x) noexcept : node(x) {}
  LinkedListIter(const Iter& x) noexcept : node(x.node) {}

  inline Ref operator*() const noexcept { return node->value; }
  inline Ptr operator->() const noexcept { return &node->value; }

  inline bool operator==(const Self& x) const noexcept { return node == x.node; }
  inline bool operator!=(const Self& x) const noexcept { return node != x.node; }

  inline Self& operator++() noexcept {
    node = node->next;
    return *this;
  }

  inline Self operator++(int) const noexcept {
    Self tmp = *this;
    node = node->next;
    return tmp;
  }
};

template <typename Tp> class LinkedList final : private UnCopyable {
public:
  using ValueType = Tp;
  using Iter      = LinkedListIter<Tp, Tp&, Tp*>;
  using ConstIter = LinkedListIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
  using Ptr       = Tp*;
  using ConstPtr  = const Tp*;
private:
  using NodeType  = LinkedListNode<Tp>;
  using Alloc     = SimpleAlloc<NodeType>;

  sz_t size_{};
  NodeType* head_{};
  NodeType* tail_{};

  NodeType* create_aux(const ValueType& value) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, value);
      node->next = nullptr;
    }
    catch (...) {
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  NodeType* create_aux(ValueType&& value) {
    NodeType* node = SimpleAlloc<NodeType>::allocate();
    try {
      construct(&node->value, std::move(value));
      node->next = nullptr;
    }
    catch (...) {
      Alloc::deallocate(node);
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
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  void destroy_aux(NodeType* node) noexcept {
    destroy(&node->value);
    Alloc::deallocate(node);
  }

  void insert_aux(ConstIter pos, NodeType* node) {
    ++size_;
    if (head_ == nullptr) {
      head_ = tail_ = node;
      return;
    }

    if (pos == begin()) {
      node->next = head_;
      head_ = node;
    }
    else if (pos == end()) {
      tail_->next = node;
      tail_ = node;
    }
    else {
      node->next = pos.node->next;
      pos.node->next = node;
    }
  }
public:
  LinkedList() noexcept {}
  ~LinkedList() noexcept { clear(); }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline Ref get_head() noexcept { return head_->value; }
  inline ConstRef get_head() const noexcept { return head_->value; }
  inline Ref get_tail() noexcept { return tail_->value; }
  inline ConstRef get_tail() const noexcept { return tail_->value; }
  inline Iter begin() noexcept { return Iter(head_); }
  inline ConstIter begin() const noexcept { return ConstIter(head_); }
  inline Iter end() noexcept { return Iter(); }
  inline ConstIter end() const noexcept { return ConstIter(); }

  void clear() {
    while (head_ != nullptr) {
      auto* node = head_;
      head_ = head_->next;
      destroy_aux(node);
    }
    tail_ = nullptr;
    size_ = 0;
  }

  inline void append(const ValueType& x) { insert_aux(end(), create_aux(x)); }
  inline void append(ValueType&& x) { insert_aux(end(), create_aux(std::move(x))); }

  template <typename... Args> inline void append(Args&&... args) {
    insert_aux(end(), create_aux(std::forward<Args>(args)...));
  }

  inline void append_head(const ValueType& x) {
    insert_aux(begin(), create_aux(x));
  }

  inline void append_head(ValueType&& x) {
    insert_aux(begin(), create_aux(std::move(x)));
  }

  template <typename... Args> inline void append_head(Args&&... args) {
    insert_aux(begin(), create_aux(std::forward<Args>(args)...));
  }

  ValueType pop_head() {
    NodeType* node = head_;
    if (head_ = head_->next; head_ == nullptr)
      tail_ = nullptr;
    ValueType r = node->value;
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
