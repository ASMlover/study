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
    if (x == root)
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

  x->update_height();
  y->update_height();

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

  x->update_height();
  y->update_height();

  return y;
}

inline BasePtr avlnode_fix_left(BasePtr node, BasePtr& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //       [d]  b              [d]  c               /   \
  //           / \                 / \             a     b
  //          c  [g]              e   b           / \   / \
  //         / \                     / \        [d]  e f  [g]
  //        e   f                   f  [g]

  BasePtr right = node->right;
  if (right->lheight() > right->rheight())
    avlnode_rotate_right(right, root);
  node = avlnode_rotate_left(node, root);

  return node;
}

inline BasePtr avlnode_fix_right(BasePtr node, BasePtr& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //        b  [g]              c  [g]              /   \
  //       / \                 / \                 b     a
  //     [d]  c               b   f               / \   / \
  //         / \             / \                [d]  e f  [g]
  //        e   f          [d]  e

  BasePtr left = node->left;
  if (left->lheight() < left->rheight())
    avlnode_rotate_left(left, root);
  node = avlnode_rotate_right(node, root);

  return node;
}

inline void avlnode_rebalance(BasePtr node, BasePtr& root) noexcept {
  while (node != root) {
    int lh = node->lheight();
    int rh = node->rheight();
    int diff = lh - rh;
    int height = Xt::max(lh, rh) + 1;

    if (node->height != height)
      node->height = height;
    else if (diff >= -1 && diff <= 1)
      break;

    if (diff <= -2)
      node = avlnode_fix_left(node, root);
    else if (diff >= 2)
      node = avlnode_fix_right(node, root);

    node = node->parent;
  }
}

inline void avlnode_insert(
    bool insert_left, BasePtr x, BasePtr p, Base& header) noexcept {
  BasePtr& root = header.parent;

  x->parent = p;
  x->left = x->right = nullptr;
  x->height = 0;

  if (insert_left) {
    p->left = x;
    if (p == &header)
      header.parent = header.right = x;
    else if (p == header.left)
      header.left = x;
  }
  else {
    p->right = x;
    if (p == header.right)
      header.right = x;
  }
  avlnode_rebalance(x, root);
}

inline void avlnode_erase(BasePtr x, Base& header) noexcept {
  BasePtr& root = header.parent;
  BasePtr& lmost = header.left;
  BasePtr& rmost = header.right;
  BasePtr orig = x;

  BasePtr p = nullptr;
  BasePtr y = nullptr;
  if (x->left != nullptr && x->right != nullptr) {
    x = x->right;
    while (x->left != nullptr)
      x = x->left;
    y = x->right;
    p = x->parent;
    if (y != nullptr)
      y->parent = p;
    _avlnode_replace_child(x, y, p, root);
    if (x->parent == orig)
      p = x;
    x->parent = orig->parent;
    x->left = orig->left;
    x->right = orig->right;
    x->height = orig->height;
    _avlnode_replace_child(orig, x, x->parent, root);
    x->left->parent = x;
    if (x->right != nullptr)
      x->right->parent = x;
  }
  else {
    y = x->left != nullptr ? x->left : x->right;
    p = x->parent;
    _avlnode_replace_child(x, y, p, root);
    if (y != nullptr)
      y->parent = p;
  }
  if (p != nullptr)
    avlnode_rebalance(p, root);

  if (root == orig)
    root = y;
  else if (orig->parent->left == orig)
    orig->parent->left = y;
  else
    orig->parent->right = y;

  if (lmost == orig || lmost == nullptr)
    lmost = root != nullptr ? Base::_minimum(root) : &header;
  if (rmost == orig || rmost == nullptr)
    rmost = root != nullptr ? Base::_maximum(root) : &header;
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
    if (_node->parent->parent == _node && _node->height == 0xff) {
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

  inline Link _root_link() noexcept { return static_cast<Link>(head_.parent); }
  inline Link _tail_link() noexcept { return static_cast<Link>(&head_); }
  inline Link _lmost() noexcept { return static_cast<Link>(head_.left); }
  inline Link _rmost() noexcept { return static_cast<Link>(head_.right); }

  inline void initialize() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = 0xff;
  }

  inline Link create_node(const ValueType& x) {
    Link tmp = Alloc::allocate();
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
    Link x = _root_link();
    Link p = _tail_link();
    while (x != nullptr) {
      if (value == x->value)
        return;

      p = x;
      x = value < x->value ? _left(x) : _right(x);
    }

    bool insert_left = x != nullptr || p == _tail_link() || value < p->value;
    avlnode_insert(insert_left, create_node(value), p, head_);
    ++size_;
  }

  inline void erase_aux(Link x) {
    if (empty())
      return;

    avlnode_erase(x, head_);
    destroy_node(x);
    --size_;
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

  inline void insert(const ValueType& x) { insert_aux(x); }
  inline void erase(ConstIter pos) { erase_aux(Link(pos._node)); }
};

}

void test_avl6() {
  avl6::AVLTree<int> t;
  auto show_avl = [&t] {
    std::cout << "\navl-tree size => " << t.size() << std::endl;
    if (!t.empty())
      std::cout << "avl-tree => " << t.get_head() << " <-> " << t.get_tail() << std::endl;
    for (auto i = t.begin(); i != t.end(); ++i)
      std::cout << "avl-tree item value: " << *i << std::endl;
  };

  auto rshow_avl = [&t] {
    for (auto i = --t.end(); ;) {
      std::cout << "avl-tree item value reserve: " << *(i--) << std::endl;
      if (i == t.begin()) {
        std::cout << "avl-tree item value reserve: " << *i << std::endl;
        break;
      }
    }
  };

  t.insert(56);
  t.insert(34);
  t.insert(78);
  t.insert(23);
  t.insert(9);
  t.insert(231);
  show_avl();

  t.erase(t.begin());
  show_avl();

  t.erase(--t.end());
  show_avl();

  rshow_avl();
}
