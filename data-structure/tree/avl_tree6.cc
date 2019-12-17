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

  inline int lheight() const noexcept {
    return left != nullptr ? left->height : 0;
  }

  inline int rheight() const noexcept {
    return right != nullptr ? right->height : 0;
  }

  inline void update_height() noexcept {
    height = Xt::max(lheight(), rheight()) + 1;
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

inline void _avlnode_replace_child(
    BasePtr x, BasePtr y, BasePtr p, BasePtr& root) noexcept {
  if (p != nullptr) {
    if (p->left == x)
      p->left = y;
    else
      p->right = y;
  }
  else {
    root = y;
  }
}

inline BasePtr avlnode_rotate_left(BasePtr x, BasePtr& root) noexcept {
  //          |                   |
  //          x                   y
  //           \                 / \
  //            y               x   b
  //           / \               \
  //         [a]  b              [a]

  BasePtr y = x->right;
  x->right = y->left;
  if (x->right != nullptr)
    x->right->parent = x;
  y->parent = x->parent;

  _avlnode_replace_child(x, y, x->parent, root);
  y->left = x;
  x->parent = y;

  return y;
}

inline BasePtr avlnode_rotate_right(BasePtr x, BasePtr& root) noexcept {
  //          |                   |
  //          x                   y
  //         /                   / \
  //        y                   a   x
  //       / \                     /
  //      a  [b]                 [b]

  BasePtr y = x->left;
  x->left = y->right;
  if (x->left != nullptr)
    x->left->parent = x;
  y->parent = x->parent;

  _avlnode_replace_child(x, y, x->parent, root);
  y->right = x;
  x->parent = y;

  return y;
}

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

template <typename Tp> class AVLTree final : private UnCopyable {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = AVLIter<Tp, Tp&, Tp*>;
  using ConstIter = AVLIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Node      = AVLNode<ValueType>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = Xt::SimpleAlloc<Node>;

  SizeType size_{};
  Node head_{};

  static inline Link _parent(BasePtr x) noexcept { return static_cast<Link>(x->parent); }
  static inline Link _parent(Link x) noexcept { return static_cast<Link>(x->parent); }
  static inline Link _left(BasePtr x) noexcept { return static_cast<Link>(x->left); }
  static inline Link _left(Link x) noexcept { return static_cast<Link>(x->left); }
  static inline Link _right(BasePtr x) noexcept { return static_cast<Link>(x->right); }
  static inline Link _right(Link x) noexcept { return static_cast<Link>(x->right); }

  inline Link _root_link() noexcept { return static_cast<Link>(head_.left); }
  inline Link _tail_link() noexcept { return static_cast<Link>(&head_); }

  inline void initialize() noexcept {
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = 0;
  }

  inline Link create_node(const ValueType& x) {
    Link tmp = Alloc::create_node();
    try {
      Xt::construct(&tmp->value, x);
    }
    catch (...) {
      Alloc::deallocate(tmp);
      throw;
    }
    return tmp;
  }

  inline void destroy_node(Link p) {
    Xt::destroy(&p->value);
    Alloc::deallocate(p);
  }

  inline void insert_aux(const ValueType& value) {
  }
public:
  AVLTree() noexcept {
    initialize();
  }

  ~AVLTree() noexcept {
  }

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

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }
};

}

void test_avl6() {
  avl6::AVLIter<int, int&, int*> i;
  avl6::AVLTree<int> t;
}
