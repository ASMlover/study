// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TECH_BASIC_STACK_HEADER_H__
#define __TECH_BASIC_STACK_HEADER_H__

namespace tech {

template <typename T>
struct StackNode {
  struct StackNode* next;
  T                 value;
};

template <typename T>
class Stack : UnCopyable {
  StackNode<T>* head_;
public:
  Stack(void)
    : head_(nullptr) {
  }

  ~Stack(void) {
    clear();
  }

  void clear(void) {
    StackNode<T>* node;
    while (nullptr != head_) {
      node = head_;
      head_ = head_->next;
      delete node;
    }
    head_ = nullptr;
  }

  void push(const T& value) {
    StackNode<T>* node = new StackNode<T>();
    node->next = head_;
    node->value = value;
    head_ = node;
  }

  void pop(void) {
    StackNode<T>* node = head_;
    head_ = head_->next;

    delete node;
  }

  T& top(void) {
    return head_->value;
  }
};

}

#endif  // __TECH_BASIC_STACK_HEADER_H__
