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
#include <algorithm>
#include <utility>
#include "common.hh"

namespace avl4 {

template <typename Tp> inline void construct(Tp* p) noexcept {
  new ((void*)p) Tp();
}

template <typename Tp1, typename Tp2>
inline void construct(Tp1* p, const Tp2& v) noexcept {
  new ((void*)p) Tp2(v);
}

template <typename Tp1, typename Tp2>
inline void construct(Tp1* p, Tp2&& v) noexcept {
  new ((void*)p) Tp2(std::forward<Tp2>(v));
}

template <typename Tp, typename... Args>
inline void construct(Tp* p, Args&&... args) noexcept {
  new ((void*)p) Tp(std::forward<Args>(args)...);
}

template <typename Tp> inline void destroy(Tp* p) noexcept {
  p->~Tp();
}

template <typename Tp> class AVLAlloc final : private UnCopyable {
public:
  static Tp* allocate() noexcept {
    return (Tp*)std::malloc(sizeof(Tp));
  }

  static void deallocate(Tp* p) noexcept {
    std::free(p);
  }
};

struct AVLNodeBase {
  using BasePtr       = AVLNodeBase*;
  using ConstBasePtr  = const AVLNodeBase*;

  BasePtr parent;
  BasePtr left;
  BasePtr right;
  int bal_factor;

  inline BasePtr get_minimum() noexcept {
    BasePtr x = this;
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  inline ConstBasePtr get_minimum() const noexcept {
    ConstBasePtr x = this;
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  inline BasePtr get_maximum() noexcept {
    BasePtr x = this;
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  inline ConstBasePtr get_maximum() const noexcept {
    ConstBasePtr x = this;
    while (x->right != nullptr)
      x = x->right;
    return x;
  }
};

inline AVLNodeBase* avlnode_inc(AVLNodeBase* x) noexcept {
  if (x->right != nullptr) {
    x = x->right;
    while (x->left != nullptr)
      x = x->left;
  }
  else {
    AVLNodeBase* p = x->parent;
    while (x == p->right) {
      x = p;
      p = p->parent;
    }
    if (x->right != p)
      x = p;
  }
  return x;
}

inline AVLNodeBase* avlnode_dec(AVLNodeBase* x) noexcept {
  if (x->parent->parent == x && x->bal_factor == -2) {
    x = x->right;
  }
  else if (x->left != nullptr) {
    x = x->left;
    while (x->right != nullptr)
      x = x->right;
  }
  else {
    AVLNodeBase* p = x->parent;
    while (x == p->left) {
      x = p;
      p = p->parent;
    }
    x = p;
  }
  return x;
}

inline void avlnode_rotate_left(AVLNodeBase* x, AVLNodeBase*& root) noexcept {
  //          |                   |
  //          x                   y
  //           \                 / \
  //            y               x   b
  //           / \               \
  //         [a]  b              [a]

  AVLNodeBase* y = x->right;
  x->right = y->left;
  y->left = x;
  y->parent = x->parent;
  x->parent = y;
  if (x->right != nullptr)
    x->right->parent = x;

  if (x == root) {
    root = y;
  }
  else {
    if (y->parent->left == x)
      y->parent->left = y;
    else
      y->parent->right = y;
  }

  // reset the balancing factor
  if (y->bal_factor == 1) {
    x->bal_factor = y->bal_factor = 0;
  }
  else {
    x->bal_factor = 1;
    y->bal_factor = -1;
  }
}

inline void avlnode_rotate_right(AVLNodeBase* x, AVLNodeBase*& root) noexcept {
  //          |                   |
  //          x                   y
  //         /                   / \
  //        y                   a   x
  //       / \                     /
  //      a  [b]                 [b]

  AVLNodeBase* y = x->left;
  x->left = y->right;
  y->right = x;
  y->parent = x->parent;
  x->parent = y;
  if (x->left != nullptr)
    x->left->parent = x;

  if (x == root) {
    root = y;
  }
  else {
    if (y->parent->left == x)
      y->parent->left = y;
    else
      y->parent->right = y;
  }

  // reset the balancing factor
  if (y->bal_factor == -1) {
    x->bal_factor = y->bal_factor = 0;
  }
  else {
    x->bal_factor = -1;
    y->bal_factor = 1;
  }
}

inline void avlnode_rotate_left_right(
    AVLNodeBase* a, AVLNodeBase*& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //        b  [g]              c  [g]              /   \
  //       / \                 / \                 b     a
  //     [d]  c               b   f               / \   / \
  //         / \             / \                [d]  e f  [g]
  //        e   f          [d]  e

  AVLNodeBase* b = a->left;
  AVLNodeBase* c = b->right;
  a->left = c->right;
  b->right = c->left;
  c->left = b;
  c->right = a;
  c->parent = a->parent;
  a->parent = b->parent = c;
  if (a->left != nullptr)
    a->left->parent = a;
  if (b->right != nullptr)
    b->right->parent = b;

  if (a == root) {
    root = c;
  }
  else {
    if (c->parent->left == a)
      c->parent->left = c;
    else
      c->parent->right = c;
  }

  // reset the balancing factor
  switch (c->bal_factor) {
  case -1:
    a->bal_factor = 1;
    b->bal_factor = 0;
    break;
  case 0:
    a->bal_factor = b->bal_factor = 0;
    break;
  case 1:
    a->bal_factor = 0;
    b->bal_factor = -1;
    break;
  }
  c->bal_factor = 0;
}

inline void avlnode_rotate_right_left(
    AVLNodeBase* a, AVLNodeBase*& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //       [d]  b              [d]  c               /   \
  //           / \                 / \             a     b
  //          c  [g]              e   b           / \   / \
  //         / \                     / \        [d]  e f  [g]
  //        e   f                   f  [g]

  AVLNodeBase* b = a->right;
  AVLNodeBase* c = b->left;
  a->right = c->left;
  b->left = c->right;
  c->left = a;
  c->right = b;
  c->parent = a->parent;
  a->parent = b->parent = c;
  if (a->right != nullptr)
    a->right->parent = a;
  if (b->left != nullptr)
    b->left->parent = b;

  if (a == root) {
    root = c;
  }
  else {
    if (c->parent->left == a)
      c->parent->left = c;
    else
      c->parent->right = c;
  }

  // reset the balancing factor
  switch (c->bal_factor) {
  case -1:
    a->bal_factor = 0;
    b->bal_factor = 1;
    break;
  case 0:
    a->bal_factor = b->bal_factor = 0;
    break;
  case 1:
    a->bal_factor = -1;
    b->bal_factor = 0;
    break;
  }
  c->bal_factor = 0;
}

inline void avltree_insert(bool insert_left,
    AVLNodeBase* x, AVLNodeBase* p, AVLNodeBase& header) noexcept {
  AVLNodeBase*& root = header.parent;

  // initialize the new node
  x->parent = p;
  x->left = x->right = nullptr;
  x->bal_factor = 0;

  // insert: first node is always insert into left
  if (insert_left) {
    p->left = x;
    if (p == &header)
      header.parent = header.right = x;
    else if (p == header.left)
      header.left = x; // maintian leftmost pointing to the minimum node
  }
  else {
    p->right = x;
    if (p == header.right)
      header.right = x; // maintain rightmost pointing to the maximum node
  }

  // rebalance the factor
  while (x != root) {
    switch (x->parent->bal_factor) {
    case 0:
      x->parent->bal_factor = x == x->parent->left ? -1 : 1;
      x = x->parent;
      break;
    case 1:
      if (x == x->parent->left) {
        x->parent->bal_factor = 0;
      }
      else {
        if (x->bal_factor == -1)
          avlnode_rotate_right_left(x->parent, root);
        else
          avlnode_rotate_left(x->parent, root);
      }
      return;
    case -1:
      if (x == x->parent->left) {
        if (x->bal_factor == 1)
          avlnode_rotate_left_right(x->parent, root);
        else
          avlnode_rotate_right(x->parent, root);
      }
      else {
        x->parent->bal_factor = 0;
      }
      return;
    }
  }
}

inline AVLNodeBase* avltree_erase(
    AVLNodeBase* z, AVLNodeBase& header) noexcept {
  AVLNodeBase*& root = header.parent;
  AVLNodeBase*& leftmost = header.left;
  AVLNodeBase*& rightmost = header.right;
  AVLNodeBase* y = z;
  AVLNodeBase* x = nullptr;
  AVLNodeBase* x_parent = nullptr;

  if (y->left == nullptr) {
    x = y->right;
  }
  else if (y->right == nullptr) {
    x = y->left;
  }
  else {
    y = y->right;
    while (y->left != nullptr)
      y = y->left;
    x = y->right;
  }

  if (y != z) {
    z->left->parent = y;
    y->left = z->left;
    if (y != z->right) {
      x_parent = y->parent;
      if (x != nullptr)
        x->parent = y->parent;
      y->parent->left = x;
      y->right = z->right;
      z->right->parent = y;
    }
    else {
      x_parent = y;
    }

    if (root == z)
      root = y;
    else if (z->parent->left == z)
      z->parent->left = y;
    else
      z->parent->right = y;
    y->parent = z->parent;
    y->bal_factor = z->bal_factor;
    y = z;
  }
  else {
    x_parent = y->parent;
    if (x != nullptr)
      x->parent = y->parent;

    if (root == z) {
      root = x;
    }
    else {
      if (z->parent->left == z)
        z->parent->left = x;
      else
        z->parent->right = x;
    }

    if (leftmost == z) {
      if (z->right == nullptr)
        leftmost = z->parent;
      else
        leftmost = x->get_minimum();
    }
    if (rightmost == z) {
      if (z->left == nullptr)
        rightmost = z->parent;
      else
        rightmost = x->get_maximum();
    }
  }

  // rebalancing
  while (x != root) {
    switch (x_parent->bal_factor) {
    case 0:
      x_parent->bal_factor = x == x_parent->right ? -1 : 1;
      return y;
    case -1:
      if (x == x_parent->left) {
        x_parent->bal_factor = 0;
        x = x_parent;
        x_parent = x_parent->parent;
      }
      else {
        AVLNodeBase* a = x_parent->left;
        if (a->bal_factor == 1)
          avlnode_rotate_left_right(x_parent, root);
        else
          avlnode_rotate_right(x_parent, root);
        x = x_parent->parent;
        x_parent = x->parent;

        if (x->bal_factor == 1)
          return y;
      }
      break;
    case 1:
      if (x == x_parent->right) {
        x_parent->bal_factor = 0;
        x = x_parent;
        x_parent = x_parent->parent;
      }
      else {
        AVLNodeBase* a = x_parent->right;
        if (a->bal_factor == -1)
          avlnode_rotate_right_left(x_parent, root);
        else
          avlnode_rotate_left(x_parent, root);
        x = x_parent->parent;
        x_parent = x->parent;

        if (x->bal_factor == -1)
          return y;
      }
      break;
    }
  }
  return y;
}

template <typename Tp> struct AVLNode : public AVLNodeBase {
  Tp value;
};

template <typename _Tp, typename _Ptr, typename _Ref> struct AVLIter {
  using Iter      = AVLIter<_Tp, _Tp*, _Tp&>;
  using Self      = AVLIter<_Tp, _Ptr, _Ref>;
  using ValueType = _Tp;
  using Ptr       = _Ptr;
  using Ref       = _Ref;
  using Node      = AVLNode<_Tp>;

  Node* node{};

  AVLIter() noexcept {}
  AVLIter(AVLNodeBase* x) noexcept : node(static_cast<Node*>(x)) {}
  AVLIter(Node* x) noexcept : node(x) {}
  AVLIter(const Node* x) noexcept : node(x) {}
  AVLIter(const Iter& x) noexcept : node(x.node) {}

  inline bool operator==(const Self& x) const noexcept { return node == x.node; }
  inline bool operator!=(const Self& x) const noexcept { return node != x.node; }

  inline Ref operator*() const noexcept { return node->value; }
  inline Ptr operator->() const noexcept { return &node->value; }

  inline Self& operator++() noexcept {
    node = static_cast<Node*>(avlnode_inc(node));
    return *this;
  }

  inline Self operator++(int) noexcept {
    Self tmp(*this);
    node = static_cast<Node*>(avlnode_inc(node));
    return tmp;
  }

  inline Self& operator--() noexcept {
    node = static_cast<Node*>(avlnode_dec(node));
    return *this;
  }

  inline Self operator--(int) noexcept {
    Self tmp(*this);
    node = static_cast<Node*>(avlnode_dec(node));
    return tmp;
  }
};

template <typename Tp> class AVLTree final : private UnCopyable {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = AVLIter<Tp, Tp*, Tp&>;
  using ConstIter = AVLIter<Tp, const Tp*, const Tp&>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Node      = AVLNode<ValueType>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = AVLAlloc<Node>;

  SizeType size_{};
  Node head_{};

  inline void initialize() noexcept {
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.bal_factor = -2;
  }

  inline Link _head_link() noexcept { return static_cast<Link>(head_.parent); }
  inline ConstLink _head_link() const noexcept { return static_cast<ConstLink>(head_.parent); }
  inline Link _tail_link() noexcept { return static_cast<Link>(&head_); }
  inline ConstLink _tail_link() const noexcept { return static_cast<ConstLink>(&head_); }

  static Link _parent(Link x) noexcept { return static_cast<Link>(x->parent); }
  static ConstLink _parent(ConstLink x) noexcept { return static_cast<ConstLink>(x->parent); }
  static Link _left(Link x) noexcept { return static_cast<Link>(x->left); }
  static ConstLink _left(ConstLink x) noexcept { return static_cast<ConstLink>(x->left); }
  static Link _right(Link x) noexcept { return static_cast<Link>(x->right); }
  static ConstLink _right(ConstLink x) noexcept { return static_cast<ConstLink>(x->right); }

  void _erase_all(Node* x) {
    while (x != nullptr) {
      _erase_all(_right(x));
      Node* y = _left(x);
      destroy_node(x);
      x = y;
    }
  }

  inline Node* create_node(const ValueType& x) {
    Node* node = Alloc::allocate();
    try {
      construct(&node->value, x);
    }
    catch (...) {
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  inline void destroy_node(Node* node) {
    destroy(&node->value);
    Alloc::deallocate(node);
  }

  inline void insert_aux(Node* node) {
    Link x = _head_link();
    Link y = _tail_link();
    while (x != nullptr) {
      y = x;
      x = node->value < x->value ? _left(x) : _right(x);
    }

    bool insert_left = x != nullptr || y == _tail_link() || node->value < y->value;
    avltree_insert(insert_left, node, y, head_);
    ++size_;
  }

  inline void erase_aux(Node* node) {
    Node* y = static_cast<Node*>(avltree_erase(node, head_));
    destroy_node(y);
    --size_;
  }

  inline ConstLink find_aux(const ValueType& k) const noexcept {
    ConstLink x = _head_link();
    ConstLink y = _tail_link();
    while (x != nullptr) {
      if (k == x->value) {
        y = x;
        break;
      }
      else {
        x = k < x->value ? _left(x) : _right(x);
      }
    }
    return y;
  }
public:
  AVLTree() noexcept {
    initialize();
  }

  ~AVLTree() noexcept {
    clear();
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

  inline void clear() {
    _erase_all(_head_link());
    initialize();
  }

  inline void insert(const ValueType& x) { insert_aux(create_node(x)); }

  inline void erase(ConstIter pos) { erase_aux(pos.node); }

  inline Iter find(const ValueType& k) noexcept {
    return Iter(const_cast<Node*>(find_aux(k)));
  }

  inline ConstIter find(const ValueType& k) const noexcept {
    return ConstIter(find_aux(k));
  }
};

}

void test_avl4() {
  avl4::AVLTree<int> t;

  auto show_avl = [&t] {
    std::cout << "avl-tree -> size: "
      << t.size() << ", empty: " << t.empty() << std::endl;
    for (auto i = t.begin(); i != t.end(); ++i)
      std::cout << "avl-tree item value -> " << *i << std::endl;
  };

  t.insert(34);
  t.insert(56);
  t.insert(33);
  t.insert(23);
  t.insert(9);
  t.insert(4);
  t.insert(222);
  show_avl();

  std::cout << "find 33: " << (t.find(33) != t.end()) << std::endl;

  t.erase(t.find(33));
  show_avl();
}
