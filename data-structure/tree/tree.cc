// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <iostream>
#include "common.hh"

namespace tree {

using ColorType = bool;
static constexpr ColorType kColorRed = false;
static constexpr ColorType kColorBlack = true;
static constexpr int kHeightMask = 0xff;

struct NodeBase;
using BasePtr       = NodeBase*;
using ConstBasePtr  = const NodeBase*;

struct NodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;

  static inline BasePtr minimum(BasePtr x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  static inline ConstBasePtr minimum(ConstBasePtr x) noexcept {
    return minimum(const_cast<BasePtr>(x));
  }

  static inline BasePtr maximum(BasePtr x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  static inline ConstBasePtr maximum(ConstBasePtr x) noexcept {
    return maximum(const_cast<BasePtr>(x));
  }

  static BasePtr successor(BasePtr x) noexcept {
    if (x->right != nullptr) {
      x = x->right;
      while (x->left != nullptr)
        x = x->left;
    }
    else {
      BasePtr y = x->parent;
      while (x == y->right) {
        x = y;
        y = y->parent;
      }
      if (x->right != y)
        x = y;
    }
    return x;
  }

  static ConstBasePtr successor(ConstBasePtr x) noexcept {
    return successor(const_cast<BasePtr>(x));
  }

  static BasePtr predecessor(BasePtr x) noexcept {
    if (x->left != nullptr) {
      x = x->left;
      while (x->right != nullptr)
        x = x->right;
    }
    else {
      BasePtr y = x->parent;
      while (x == y->left) {
        x = y;
        y = y->parent;
      }
      x = y;
    }
    return x;
  }

  static ConstBasePtr predecessor(ConstBasePtr x) noexcept {
    return predecessor(const_cast<BasePtr>(x));
  }
};

struct AVLNodeBase : public NodeBase {
  using Ptr = AVLNodeBase*;

  int height;

  inline int lheight() const noexcept { return left ? Ptr(left)->height : 0; }
  inline int rheight() const noexcept { return right ? Ptr(right)->height : 0; }
  inline void update_height() noexcept { height = Xt::max(lheight(), rheight()) + 1; }

  static BasePtr predecessor(BasePtr x) noexcept {
    return (Ptr(x)->height == kHeightMask && x->parent->parent == x)
      ?  x->right : NodeBase::predecessor(x);
  }

  static ConstBasePtr predecessor(ConstBasePtr x) noexcept {
    return predecessor(const_cast<BasePtr>(x));
  }
};

struct RBNodeBase : public NodeBase {
  using Ptr = RBNodeBase*;

  ColorType color;

  inline bool is_red() const noexcept { return color == kColorRed; }
  inline bool is_black() const noexcept { return color == kColorBlack; }
  inline void set_red() noexcept { color = kColorRed; }
  inline void set_black() noexcept { color = kColorBlack; }

  static BasePtr predecessor(BasePtr x) noexcept {
    return (Ptr(x)->is_red() && x->parent->parent == x)
      ? x->right : NodeBase::predecessor(x);
  }

  static ConstBasePtr predecessor(ConstBasePtr x) noexcept {
    return predecessor(const_cast<BasePtr>(x));
  }
};

}

void test_tree() {
}
