// Copyright (c) 2017 ASMlover. All rights reserved.
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

#include <atomic>

namespace lock_free {

template <typename T>
class Stack {
  struct _Node {
    T* value{};
    _Node* next{};

    explicit _Node(T* p)
      : value(p) {
    }

    ~_Node(void) {
      delete value;
    }
  };

  std::atomic<_Node*> head_{};
public:
  Stack(void) {
  }

  ~Stack(void) {
    _Node* head = head_.load();
    _Node* node = head;
    while (head != nullptr) {
      head = head->next;
      delete node;
      node = head;
    }
  }

  void push(_Node* node) {
    if (node == nullptr)
      return;

    do {
      node->next = head_.load(std::memory_order_relaxed);
    } while (!head_.compare_exchange_weak(node->next,
          node, std::memory_order_release, std::memory_order_relaxed));
  }

  void push(T* x) {
    push(new _Node(x));
  }

  _Node* pop(void) {
    _Node* head = head_.load(std::memory_order_relaxed);
    if (head == nullptr)
      return nullptr;

    while (!head_.compare_exchange_weak(head, head->next,
          std::memory_order_release, std::memory_order_relaxed)) {
      head = head_.load(std::memory_order_relaxed);
    }
    return head;
  }
};

}
