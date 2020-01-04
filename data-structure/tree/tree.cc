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

  template <typename Checker>
  static BasePtr predecessor(BasePtr x, Checker&& checker) noexcept {
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

  static ConstBasePtr predecessor(ConstBasePtr x) noexcept {
    return predecessor(const_cast<BasePtr>(x));
  }
};

struct AVLNodeBase : public NodeBase {
  int height;

  inline bool is_root() const noexcept {
    return height == kHeightMask && parent->parent == this;
  }

  inline int lheight() const noexcept {
    return left ? static_cast<AVLNodeBase*>(left)->height : 0;
  }

  inline int rheight() const noexcept {
    return right ? static_cast<AVLNodeBase*>(right)->height : 0;
  }

  inline void update_height() noexcept {
    height = Xt::max(lheight(), rheight()) + 1;
  }
};

struct RBNodeBase : public NodeBase {
  ColorType color;

  inline bool is_root() const noexcept {
    return color == kColorRed && parent->parent == this;
  }

  inline bool is_red() const noexcept { return color == kColorRed; }
  inline bool is_black() const noexcept { return color == kColorBlack; }
  inline void set_red() noexcept { color = kColorRed; }
  inline void set_black() noexcept { color = kColorBlack; }
};

namespace details {
  inline void transplant(BasePtr u, BasePtr v, BasePtr& root) noexcept {
    if (u->parent == nullptr)
      root = v;
    else if (u->parent->left = u)
      u->parent->left = v;
    else
      u->parent->right = v;
  }

  inline BasePtr left_rotate(BasePtr x, BasePtr& root) noexcept {
    //        |                   |
    //        x                   y
    //         \                 / \
    //          y               x   b
    //         / \               \
    //        a   b               a

    BasePtr y = x->right;
    x->right = y->left;
    if (x->right != nullptr)
      x->right->parent = x;
    y->parent = x->parent;
    transplant(x, y, root);
    y->left = x;
    x->parent = y;

    return y;
  }

  inline BasePtr right_rotate(BasePtr x, BasePtr& root) noexcept {
    //        |                   |
    //        x                   y
    //       /                   / \
    //      y                   a   x
    //     / \                     /
    //    a   b                   b

    BasePtr y = x->left;
    x->left = y->right;
    if (x->left != nullptr)
      x->left->parent = x;
    y->parent = x->parent;
    transplant(x, y, root);
    y->right = x;
    x->parent = y;

    return y;
  }
}

template <typename Value> struct AVLNode : public AVLNodeBase {
  Value value;
};

template <typename Value> struct RBNode : public RBNodeBase {
  Value value;
};

struct IterBase {
  NodeBase* _node{};

  IterBase() noexcept {}
  IterBase(BasePtr x) noexcept : _node(x) {}
  IterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  inline bool operator==(const IterBase& x) const noexcept {
    return _node == x._node;
  }

  inline bool operator!=(const IterBase& x) const noexcept {
    return _node != x._node;
  }

  inline void increment() noexcept { _node = NodeBase::successor(_node); }

  template <typename Function> inline void decrement(Function&& fn) noexcept {
    _node = NodeBase::predecessor(_node, std::move(fn));
  }
};

template <typename _Tp, typename _Ref, typename _Ptr, typename _Node>
struct Iterator : public IterBase {
  using Iter = Iterator<_Tp, _Tp&, _Tp*, _Node>;
  using Self = Iterator<_Tp, _Ref, _Ptr, _Node>;
  using Ref  = _Ref;
  using Ptr  = _Ptr;
  using Link = _Node*;

  Iterator() noexcept {}
  Iterator(BasePtr x) noexcept : IterBase(x) {}
  Iterator(ConstBasePtr x) noexcept : IterBase(x) {}
  Iterator(const Iter& x) noexcept : IterBase(x._node) {}

  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }

  Self& operator++() noexcept { increment(); return *this; }
  Self operator++(int) noexcept { Self tmp(*this); increment(); return tmp; }

  Self& operator--() noexcept {
    decrement([](BasePtr x) -> bool { return Link(x)->is_root(); });
    return *this;
  }

  Self operator--(int) noexcept {
    Self tmp(*this);
    decrement([](BasePtr x) -> bool { return Link(x)->is_root(); });
    return tmp;
  }
};

template <typename Tp, typename Node> class TreeBase {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = Iterator<Tp, Tp&, Tp*, Node>;
  using ConstIter = Iterator<Tp, const Tp&, const Tp*, Node>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
protected:
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = Xt::SimpleAlloc<Node>;

  SizeType size_{};
  Node head_{};

  static inline Link _parent(BasePtr x) noexcept { return Link(x->parent); }
  static inline ConstLink _parent(ConstBasePtr x) noexcept { return ConstLink(x->parent); }
  static inline Link _left(BasePtr x) noexcept { return Link(x->left); }
  static inline ConstLink _left(ConstBasePtr x) noexcept { return ConstLink(x->left); }
  static inline Link _right(BasePtr x) noexcept { return Link(x->right); }
  static inline ConstLink _right(ConstBasePtr x) noexcept { return ConstLink(x->right); }

  inline Link& root() noexcept { return (Link&)head_.parent; }
  inline ConstLink& root() const noexcept { return (ConstLink&)head_.parent; }
  inline Link& lmost() noexcept { return (Link&)head_.left; }
  inline ConstLink& lmost() const noexcept { return (ConstLink&)head_.left; }
  inline Link& rmost() noexcept { return (Link&)head_.right; }
  inline ConstLink& rmost() const noexcept { return (ConstLink&)head_.right; }
public:
  inline bool empty() const noexcept { return size_ == 0; }
  inline SizeType size() const noexcept { return size_; }
  inline Iter begin() noexcept { return Iter(head_.left); }
  inline ConstIter begin() const noexcept { return ConstIter(head_.left); }
  inline Iter end() noexcept { return Iter(&head_); }
  inline ConstIter end() const noexcept { return ConstIter(&head_); }
  inline Ref get_head() noexcept { return *begin(); }
  inline ConstRef get_head() const noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(--end()); }
  inline ConstRef get_tail() const noexcept { return *(--end()); }

  template <typename Function> inline void for_each(Function&& fn) {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }
};

template <typename Tp>
class AVLTree final : public TreeBase<Tp, AVLNode<Tp>>, private UnCopyable {
  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = kHeightMask;
  }
public:
  AVLTree() noexcept { init(); }
  ~AVLTree() noexcept { clear(); }

  void clear() {
  }
};

}

template <typename Tree> void show_tree(Tree& t, const std::string& s) {
  std::cout << "\n" << s << " size: " << t.size() << std::endl;
  if (!t.empty()) {
    std::cout << s << "{" << t.get_head() << ", " << t.get_tail() << "}" << std::endl;
  }
  for (auto i = t.begin(); i != t.end(); ++i)
    std::cout << s << " item value: " << *i << std::endl;
}

void test_tree() {
  tree::AVLTree<int> t;
  show_tree(t, "avltree");
}
