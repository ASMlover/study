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

namespace wrencc::linked_list {

template <typename Tp> struct ListNode {
  ListNode<Tp>* next;
  Tp value;
};

template <typename _Tp, typename _Ref, typename _Ptr> struct ListIter {
  using Iter      = ListIter<_Tp, _Tp&, _Tp*>;
  using Self      = ListIter<_Tp, _Ref, _Ptr>;
  using Ref       = _Ref;
  using Ptr       = _Ptr;
  using Link      = ListNode<_Tp>*;
  using ConstLink = const ListNode<_Tp>*;

  Link _node{};

  ListIter() noexcept {}
  ListIter(Link x) noexcept : _node(x) {}
  ListIter(ConstLink x) noexcept : _node(const_cast<Link>(x)) {}
  ListIter(const Iter& x) noexcept : _node(x._node) {}

  inline bool operator==(const Self& r) const noexcept {
    return _node == r._node;
  }

  inline bool operator!=(const Self& r) const noexcept {
    return _node != r._node;
  }

  inline Link node() const noexcept { return _node; }
  inline Ref operator*() const noexcept { return _node->value; }
  inline Ptr operator->() const noexcept { return &_node->value; }

  inline Self& operator++() noexcept {
    _node = _node->next;
    return *this;
  }

  inline Self operator++(int) noexcept {
    Self tmp(*this);
    _node = _node->next;
    return tmp;
  }
};

}

namespace wrencc {

template <typename Tp> class LinkedList final : private UnCopyable {
public:
  using ValueType = Tp;
  using Iter      = linked_list::ListIter<Tp, Tp&, Tp*>;
  using ConstIter = linked_list::ListIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
  using Ptr       = Tp*;
  using ConstPtr  = const Tp*;
private:
  using Node      = linked_list::ListNode<Tp>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = SimpleAlloc<Node>;

  sz_t size_{};
  Link head_{};
  Link tail_{};

  inline Link init_node(Link p) noexcept {
    p->next = nullptr;
    return p;
  }

  Link create_node(const ValueType& value) {
    return init_node(node::create_node<Alloc, Link>(value));
  }

  Link create_node(ValueType&& value) {
    return init_node(node::create_node<Alloc, Link>(std::move(value)));
  }

  template <typename... Args> Link create_node(Args&&... args) {
    return init_node(node::create_node<Alloc, Link>(std::forward<Args>(args)...));
  }

  void destroy_node(Link p) { node::destroy_node<Alloc, Link>(p); }

  void insert_aux(ConstIter pos, Link node) {
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
      node->next = pos.node()->next;
      pos.node()->next = node;
    }
  }
public:
  LinkedList() noexcept {}
  ~LinkedList() noexcept { clear(); }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline Iter begin() noexcept { return head_; }
  inline ConstIter begin() const noexcept { return head_; }
  inline Iter end() noexcept { return Iter(); }
  inline ConstIter end() const noexcept { return ConstIter(); }
  inline Ref get_head() noexcept { return head_->value; }
  inline ConstRef get_head() const noexcept { return head_->value; }
  inline Ref get_tail() noexcept { return tail_->value; }
  inline ConstRef get_tail() const noexcept { return tail_->value; }

  void clear() {
    while (head_ != nullptr) {
      Link node = head_;
      head_ = head_->next;
      destroy_node(node);
    }
    tail_ = nullptr;
    size_ = 0;
  }

  inline void append(const ValueType& x) { insert_aux(end(), create_node(x)); }
  inline void append(ValueType&& x) { insert_aux(end(), create_node(std::move(x))); }

  template <typename... Args> inline void append(Args&&... args) {
    insert_aux(end(), create_node(std::forward<Args>(args)...));
  }

  inline void append_head(const ValueType& x) {
    insert_aux(begin(), create_node(x));
  }

  inline void append_head(ValueType&& x) {
    insert_aux(begin(), create_node(std::move(x)));
  }

  template <typename... Args> inline void append_head(Args&&... args) {
    insert_aux(begin(), create_node(std::forward<Args>(args)...));
  }

  ValueType pop_head() {
    Link node = head_;
    if (head_ = head_->next; head_ == nullptr)
      tail_ = nullptr;
    ValueType r = node->value;
    destroy_node(node);
    --size_;

    return r;
  }

  template <typename Visitor> inline void for_each(Visitor&& visitor) {
     for (auto i = begin(); i != end(); ++i)
       visitor(*i);
  }
};

}
