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
#include <tuple>
#include "common.hh"

namespace avl7 {

struct AVLNodeBase;
using BasePtr       = AVLNodeBase*;
using ConstBasePtr  = const AVLNodeBase*;

static constexpr int kHeightMask = 0xff;

struct AVLNodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;
  int height;

  inline int lheight() const noexcept { return left ? left->height : 0; }
  inline int rheight() const noexcept { return right ? right->height : 0; }
  inline void update_height() noexcept { height = Xt::max(lheight(), rheight()) + 1; }

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

inline void __avlnode_replace_child(
    BasePtr x, BasePtr y, BasePtr p, BasePtr& root) noexcept {
  if (p != nullptr) {
    if (root == x)
      root = y;
    else if (p->left == x)
      p->left = y;
    else
      p->right = y;
  }
  else {
    root = y;
  }
}

template <typename ValueType> struct AVLNode : public AVLNodeBase {
  ValueType value;
};

struct AVLIterBase {
  BasePtr _node{};

  AVLIterBase() noexcept {}
  AVLIterBase(BasePtr x) noexcept : _node(x) {}
  AVLIterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  void increment() noexcept {
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

  void decrement() noexcept {
    if (_node->height == kHeightMask && _node->parent->parent == _node) {
      _node = _node->right;
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
  using Link      = AVLNode<_Tp>*;
  using ConstLink = const AVLNode<_Tp>*;

  AVLIter() noexcept {}
  AVLIter(BasePtr x) noexcept : AVLIterBase(x) {}
  AVLIter(ConstBasePtr x) noexcept : AVLIterBase(x) {}
  AVLIter(const Iter& x) noexcept : AVLIterBase(x._node) {}

  inline Link node() noexcept { return Link(_node); }
  inline ConstLink node() const noexcept { return ConstLink(_node); }

  inline bool operator==(const Self& r) const noexcept { return _node == r._node; }
  inline bool operator!=(const Self& r) const noexcept { return _node != r._node; }
  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }
  Self& operator++() noexcept { increment(); return *this; }
  Self operator++(int) noexcept { Self tmp(*this); increment(); return tmp; }
  Self& operator--() noexcept { decrement(); return *this; }
  Self operator--(int) noexcept { Self tmp(*this); decrement(); return tmp; }
};

}

void test_avl7() {
}
