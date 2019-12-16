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
#include <iostream>
#include "common.hh"

namespace avl6 {

struct AVLNodeBase;
using Base          = AVLNodeBase;
using BasePtr       = Base*;
using ConstBasePtr  = const Base*;

struct AVLNodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;
  int height;

  inline int left_height() const noexcept {
    return left != nullptr ? left->height : 0;
  }

  inline int right_height() const noexcept {
    return right != nullptr ? right->height : 0;
  }

  inline void update_height() noexcept {
    height = Xt::max(left_height(), right_height()) + 1;
  }

  static inline BasePtr _minimum(BasePtr x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  static inline ConstBasePtr _minimum(ConstBasePtr x) noexcept {
    return _minimum(const_cast<BasePtr>(x));
  }

  static inline BasePtr _maximum(BasePtr x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  static inline ConstBasePtr _maximum(ConstBasePtr x) noexcept {
    return _maximum(const_cast<BasePtr>(x));
  }
};

template <typename Value> struct AVLNode : public AVLNodeBase {
  Value value;
};

struct AVLIterBase {
  BasePtr _node{};

  AVLIterBase() noexcept {}
  AVLIterBase(BasePtr x) noexcept : _node(x) {}
  AVLIterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  inline void increment() noexcept {
    if (_node->right != nullptr) {
      _node = _node->right;
      while (_node->left != nullptr)
        _node = _node->left;
    }
    else {
      BasePtr y = _node->parent;
      while (_node == y->right) {
        _node = y;
        y = y->parent;
      }
      if (_node->right != y)
        _node = y;
    }
  }

  inline void decrement() noexcept {
    if (_node->parent->parent == _node) {
      _node = _node->parent;
    }
    else if (_node->left != nullptr) {
      _node = _node->left;
      while (_node->right != nullptr)
        _node = _node->right;
    }
    else {
      BasePtr y = _node->parent;
      while (_node == y->left) {
        _node = y;
        y = y->parent;
      }
      _node = y;
    }
  }
};

template <typename _Tp, typename _Ref, typename _Ptr>
struct AVLIter : public AVLIterBase {
  using Iter      = AVLIter<_Tp, _Tp&, _Tp*>;
  using Self      = AVLIter<_Tp, _Ref, _Ptr>;
  using Ref       = _Ref;
  using Ptr       = _Ptr;
  using Node      = AVLNode<_Tp>;
  using Link      = Node*;
  using ConstLink = const Node*;

  AVLIter() noexcept {}
  AVLIter(BasePtr x) noexcept : AVLIterBase(x) {}
  AVLIter(ConstBasePtr x) noexcept : AVLIterBase(x) {}
  AVLIter(Link x) noexcept : AVLIterBase(x) {}
  AVLIter(ConstLink x) noexcept : AVLIterBase(x) {}
  AVLIter(const Iter& x) noexcept { _node = x._node; }

  inline bool operator==(const Self& x) const noexcept {
    return _node == x._node;
  }

  inline bool operator!=(const Self& x) const noexcept {
    return _node != x._node;
  }

  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }

  inline Self& operator++() noexcept {
    increment();
    return *this;
  }

  inline Self operator++(int) noexcept {
    Self tmp(*this);
    increment();
    return tmp;
  }

  inline Self& operator--() noexcept {
    decrement();
    return *this;
  }

  inline Self operator--(int) noexcept {
    Self tmp(*this);
    decrement();
    return tmp;
  }
};

}

void test_avl6() {
  avl6::AVLIter<int, int&, int*> i;
}
