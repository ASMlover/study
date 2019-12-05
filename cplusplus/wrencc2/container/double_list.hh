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

template <typename Tp> struct DoubleListNode {
  DoubleListNode<Tp>* prev;
  DoubleListNode<Tp>* next;
  Tp value;
};

template <typename _Tp, typename _Ref, typename _Ptr> struct DoubleListIter {
  using Iter      = DoubleListIter<_Tp, _Tp&, _Tp*>;
  using Self      = DoubleListIter<_Tp, _Ref, _Ptr>;
  using ValueType = _Tp;
  using Ref       = _Ref;
  using Ptr       = _Ptr;
  using NodeType  = DoubleListNode<_Tp>;

  NodeType* node{};

  DoubleListIter() noexcept {}
  DoubleListIter(NodeType* x) noexcept : node(x) {}
  DoubleListIter(const Iter& x) noexcept : node(x.node) {}

  inline Ref operator*() const noexcept { return node->value; }
  inline Ptr operator->() const noexcept { return &node->value; }

  inline bool operator==(const Self& x) const noexcept { return node ==  x.node; }
  inline bool operator!=(const Self& x) const noexcept { return node != x.node; }

  inline Self& operator--() noexcept {
    node = node->prev;
    return *this;
  }

  inline Self operator--(int) const noexcept {
    Self tmp = *this;
    node = node->prev;
    return tmp;
  }

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

template <typename Tp> class DoubleList final : private UnCopyable {
public:
  using ValueType = Tp;
  using Iter      = DoubleListIter<Tp, Tp&, Tp*>;
  using ConstIter = DoubleListIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp;
  using Ptr       = Tp*;
  using ConstPtr  = const Tp*;
private:
  using NodeType  = DoubleListNode<Tp>;
  using Alloc     = SimpleAlloc<NodeType>;

  sz_t size_{};
  NodeType* head_{};

  inline void initialize_head() noexcept {
    head_ = Alloc::allocate();
    head_->prev = head_;
    head_->next = head_;
  }

  inline void destroy_head() noexcept {
    Alloc::deallocate(head_);
  }

  NodeType* create_aux(const ValueType& x) {
    NodeType* node = Alloc::allocate();
    try {
      construct(&node->value, x);
      node->prev = node->prev = nullptr;
    }
    catch (...) {
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  NodeType* create_aux(ValueType&& x) {
    NodeType* node = Alloc::allocate();
    try {
      construct(&node->value, std::move(x));
      node->prev = node->prev = nullptr;
    }
    catch (...) {
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  template <typename... Args> NodeType* create_aux(Args&&... args) {
    NodeType* node = Alloc::allocate();
    try {
      construct(&node->value, std::forward<Args>(args)...);
      node->prev = node->prev = nullptr;
    }
    catch (...) {
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  void destroy_aux(NodeType* node) {
    destroy(&node->value);
    Alloc::deallocate(node);
  }

  inline void insert_aux(ConstIter pos, NodeType* node) noexcept {
    node->next = pos.node;
    node->prev = pos.node->prev;
    pos.node->prev->next = node;
    pos.node->prev = node;
    ++size_;
  }

  inline ValueType erase_aux(ConstIter pos) noexcept {
    NodeType* prev = pos.node->prev;
    NodeType* next = pos.node->next;
    prev->next = next;
    next->prev = prev;

    ValueType r = *pos;
    destroy(&pos.node->value);
    Alloc::deallocate(pos.node);
    --size_;

    return r;
  }
public:
  DoubleList() noexcept {
    initialize_head();
  }

  ~DoubleList() noexcept {
    clear();
    destroy_head();
  }

  inline bool empty() const noexcept { return head_->next == head_; }
  inline sz_t size() const noexcept { return size_; }
  inline Ref get_head() noexcept { return *begin(); }
  inline ConstRef get_head() const noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(--end()); }
  inline ConstRef get_tail() const noexcept { return *(--end()); }
  inline Iter begin() noexcept { return Iter(head_->next); }
  inline ConstIter begin() const noexcept { return ConstIter(head_->next); }
  inline Iter end() noexcept { return Iter(head_); }
  inline ConstIter end() const noexcept { return ConstIter(head_); }

  void clear() {
    NodeType* cur = head_->next;
    while (cur != head_) {
      NodeType* node = cur;
      cur = cur->next;
      destroy(&node->value);
      Alloc::deallocate(node);
    }
    head_->prev = head_->next = head_;
    size_ = 0;
  }

  inline void append(const ValueType& x) { insert_aux(end(), create_aux(x)); }
  inline void append(ValueType&& x) { insert_aux(end(), create_aux(std::move(x))); }

  template <typename... Args> inline void append(Args&&... args) {
    insert_aux(end(), create_aux(std::forward<Args>(args)...));
  }

  inline void insert(ConstIter pos, const ValueType& x) {
    insert_aux(pos, create_aux(x));
  }

  inline void insert(ConstIter pos, ValueType&& x) {
    insert_aux(pos, create_aux(std::move(x)));
  }

  template <typename... Args> inline void insert(ConstIter pos, Args&&... args) {
    insert_aux(pos, create_aux(std::forward<Args>(args)...));
  }

  inline ValueType pop_head() { return erase_aux(begin()); }
  inline ValueType pop_tail() { return erase_aux(end()); }

  inline void erase(ConstIter pos) { erase_aux(pos); }

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }
};

}
