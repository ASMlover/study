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

  inline void initialize_head_aux() noexcept {
    head_ = SimpleAlloc<NodeType>::allocate(1);
    head_->prev = head_;
    head_->next = head_;
  }

  inline void destroy_head_aux() noexcept {
    SimpleAlloc<NodeType>::deallocate(head_);
  }
public:
  DoubleList() noexcept { initialize_head_aux(); }
  ~DoubleList() noexcept {
    // clear
    destroy_head_aux();
  }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline T& get_head() noexcept { return head_->next->value; }
  inline const T& get_head() const noexcept { return head_->next->value; }
  inline T& get_tail() noexcept { return head_->prev->value; }
  inline const T& get_tail() const noexcept { return head_->prev->value; }

  void clear() {
  }

  void append(const T& x);
  void append(T&& x);
  template <typename... Args> void append(Args&&... args);

  T pop_head();
  T pop_tail();

  template <typename Function> inline void for_each(Function&& fn) noexcept;
};

}
