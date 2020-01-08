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
#pragma once

#include <tuple>
#include "common.hh"

namespace xt::tree {

using ColorType = bool;
static constexpr ColorType kColorRed = false;
static constexpr ColorType kColorBlk = true;
static constexpr int kHeightMark = 0xff;

struct NodeBase;
using BasePtr      = NodeBase*;
using ConstBasePtr = const NodeBase*;

namespace impl::avl {
  void insert(bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept;
  void erase(BasePtr x, NodeBase& header) noexcept;
}

namespace impl::rb {
  void insert(bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept;
  void erase(BasePtr x, NodeBase& header) noexcept;
}

struct NodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;

  template <typename Target> inline Target* as() noexcept {
    return static_cast<Target*>(this);
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

  template <typename HeadChecker>
  static BasePtr predecessor(BasePtr x, HeadChecker&& checker) noexcept {
    if (checker(x)) {
      x = x->right;
    }
    else if (x->left != nullptr) {
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

  template <typename HeadChecker>
  static ConstBasePtr predecessor(ConstBasePtr x, HeadChecker&& checker) noexcept {
    return predecessor(const_cast<BasePtr>(x), std::move(checker));
  }
};

struct AVLNodeBase : public NodeBase {
  int height;

  inline bool is_header() const noexcept {
    return height == kHeightMark && parent->parent == this;
  }

  inline int lheight() const noexcept {
    return left != nullptr ? left->as<AVLNodeBase>()->height : 0;
  }

  inline int rheight() const noexcept {
    return right != nullptr ? right->as<AVLNodeBase>()->height : 0;
  }

  inline void update_height() noexcept {
    height = Xt::max(lheight(), rheight()) + 1;
  }
};

struct RBNodeBase : public NodeBase {
  ColorType color;

  inline bool is_header() const noexcept {
    return is_red() && parent->parent == this;
  }

  inline bool is_red() const noexcept { return color == kColorRed; }
  inline bool is_blk() const noexcept { return color == kColorBlk; }
  inline void as_red() noexcept { color = kColorRed; }
  inline void as_blk() noexcept { color = kColorBlk; }
  inline void set_color(ColorType c = kColorRed) noexcept { color = c; }
};

template <typename Value> struct AVLNode : public AVLNodeBase {
  Value value;
};

template <typename Value> struct RBNode : public RBNodeBase {
  Value value;
};

struct TIterBase {
  NodeBase* _node{};

  TIterBase() noexcept {}
  TIterBase(BasePtr x) noexcept : _node(x) {}
  TIterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  inline bool operator==(const TIterBase& r) const noexcept {
    return _node == r._node;
  }

  inline bool operator!=(const TIterBase& r) const noexcept {
    return _node != r._node;
  }

  inline void increment() noexcept { _node = NodeBase::successor(_node); }

  template <typename HeadChecker>
  inline void decrement(HeadChecker&& checker) noexcept {
    _node = NodeBase::predecessor(_node, std::move(checker));
  }
};

}
