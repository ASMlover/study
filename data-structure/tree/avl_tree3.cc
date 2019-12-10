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

namespace avl3 {

struct AVLNode {
  AVLNode* parent{};
  AVLNode* left{};
  AVLNode* right{};
  int bal_factor{};
  int value{};

  static AVLNode* minimum(AVLNode* x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  static AVLNode* maximum(AVLNode* x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }
};

inline AVLNode* avltree_incr(AVLNode* node) noexcept {
  if (node->right != nullptr) {
    node = node->right;
    while (node->left != nullptr)
      node = node->left;
  }
  else {
    AVLNode* parent = node->parent;
    while (node == parent->right) {
      node = parent;
      parent = parent->parent;
    }
    if (node->right != parent)
      node = parent;
  }
  return node;
}

inline AVLNode* avltree_decr(AVLNode* node) noexcept {
  if (node->parent->parent == node && node->bal_factor == -2) {
    node = node->right;
  }
  else if (node->left != nullptr) {
    node = node->left;
    while (node->right != nullptr)
      node = node->right;
  }
  else {
    AVLNode* parent = node->parent;
    while (node == parent->left) {
      node = parent;
      parent = parent->parent;
    }
    node = parent;
  }
  return node;
}

template <typename _Tp, typename _Ref, typename _Ptr> struct AVLNodeIter {
  using Iter  = AVLNodeIter<_Tp, _Tp&, _Tp*>;
  using Self  = AVLNodeIter<_Tp, _Ref, _Ptr>;
  using Ref   = _Ref;
  using Ptr   = _Ptr;

  AVLNode* node{};

  AVLNodeIter() noexcept {}
  AVLNodeIter(AVLNode* n) noexcept : node(n) {}
  AVLNodeIter(const Self& x) noexcept : node(x.node) {}

  inline bool operator==(const Self& r) const noexcept { return node == r.node; }
  inline bool operator!=(const Self& r) const noexcept { return node != r.node; }

  inline Ref operator*() const noexcept { return static_cast<Ref>(node->value); }
  inline Ptr operator->() const noexcept { return static_cast<Ptr>(&node->value); }

  inline Self& operator++() noexcept {
    node = avltree_incr(node);
    return *this;
  }

  inline Self operator++(int) noexcept {
    Self tmp(*this);
    node = avltree_incr(node);
    return tmp;
  }

  inline Self& operator--() noexcept {
    node = avltree_decr(node);
    return *this;
  }

  inline Self operator--(int) noexcept {
    Self tmp(*this);
    node = avltree_decr(node);
    return tmp;
  }
};

inline void avltree_rotate_left(AVLNode* node, AVLNode*& root) noexcept {
  //        |                         |
  //        x                         y
  //       / \                       / \
  //      a   y                     x   c
  //         / \                   / \
  //        b   c                 a   b

  AVLNode* right = node->right;
  node->right = right->left;
  right->left = node;

  right->parent = node->parent;
  node->parent = right;
  if (node->right != nullptr)
    node->right->parent = node;

  if (node == root) {
    root = right;
  }
  else {
    if (right->parent->left == node)
      right->parent->left = right;
    else
      right->parent->right = right;
  }

  // reset the balancing factor
  if (right->bal_factor == 1) {
    node->bal_factor = right->bal_factor = 0;
  }
  else {
    node->bal_factor = 1;
    right->bal_factor = -1;
  }
}

inline void avltree_rotate_right(AVLNode* node, AVLNode*& root) noexcept {
  //        |                         |
  //        x                         y
  //       / \                       / \
  //      y   c                     a   x
  //     / \                           / \
  //    a   b                         b   c

  AVLNode* left = node->left;
  node->left = left->right;
  left->right = node;

  left->parent = node->parent;
  node->parent = left;
  if (node->left != nullptr)
    node->left->parent = node;

  if (node == root) {
    root = node;
  }
  else {
    if (left->parent->left == node)
      left->parent->left = left;
    else
      left->parent->right = left;
  }

  if (left->bal_factor == -1) {
    node->bal_factor = left->bal_factor = 0;
  }
  else {
    node->bal_factor = -1;
    left->bal_factor = 1;
  }
}

inline void avltree_rotate_left_right(AVLNode* a, AVLNode*& root) noexcept {
  //      |               |               |
  //      a               a               c
  //     / \             / \             / \
  //    /   g           /   g           /   \
  //   b               c               b     a
  //  / \             / \             / \   / \
  // d   c           b   f           d   e f   g
  //    / \         / \
  //   e   f       d   e

  AVLNode* b = a->left;
  AVLNode* c = b->right;

  a->left = c->right;
  b->right = c->left;

  c->right = a;
  c->left = b;
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
    if (a == c->parent->left)
      c->parent->left = c;
    else
      c->parent->right = c;
  }

  // balancing ...
  switch (c->bal_factor) {
  case -1:
    a->bal_factor = 1;
    b->bal_factor = 0;
    break;
  case 0:
    a->bal_factor = b->bal_factor = 0; break;
  case 1:
    a->bal_factor = 0;
    b->bal_factor = -1;
    break;
  }
  c->bal_factor = 0;
}

inline void avltree_rotate_right_left(AVLNode* a, AVLNode*& root) noexcept {
  //      |               |               |
  //      a               a               c
  //     / \             / \             / \
  //    /   \           /   \           /   \
  //   d     b         d     c         a     b
  //        / \             / \       / \   / \
  //       c   g           e   b     d   e f   g
  //      / \                 / \
  //     e   f               f   g

  AVLNode* b = a->right;
  AVLNode* c = b->left;

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
    if (a == c->parent->left)
      c->parent->left = c;
    else
      c->parent->right = c;
  }

  // balancing ...
  switch (c->bal_factor) {
  case -1:
    a->bal_factor = 0;
    b->bal_factor = 1;
    break;
  case 0:
    a->bal_factor = b->bal_factor = 0; break;
  case 1:
    a->bal_factor = -1;
    b->bal_factor = 0;
    break;
  }
  c->bal_factor = 0;
}

inline void avltree_insert_rebalance(
    bool insert_left, AVLNode* x, AVLNode* p, AVLNode& header) noexcept {
  AVLNode*& root = header.parent;

  x->parent = p;
  x->left = x->right = nullptr;
  x->bal_factor = 0;

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

  // rebalance ...
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
          avltree_rotate_right_left(x->parent, root);
        else
          avltree_rotate_left(x->parent, root);
      }
      return;
    case -1:
      if (x == x->parent->left) {
        if (x->bal_factor == 1)
          avltree_rotate_left_right(x->parent, root);
        else
          avltree_rotate_right(x->parent, root);
      }
      else {
        x->parent->bal_factor = 0;
      }
      return;
    }
  }
}

inline AVLNode* avltree_erase_rebalance(AVLNode* z, AVLNode& header) noexcept {
  AVLNode*& root = header.parent;
  AVLNode*& leftmost = header.left;
  AVLNode*& rightmost = header.right;
  AVLNode* y = z;
  AVLNode* x = nullptr;
  AVLNode* x_parent = nullptr;

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
        leftmost = AVLNode::minimum(x);
    }
    if (rightmost == z) {
      if (z->left == nullptr)
        rightmost = z->parent;
      else
        rightmost = AVLNode::maximum(x);
    }
  }

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
        AVLNode* a = x_parent->left;
        if (a->bal_factor == 1) {
          avltree_rotate_left_right(x_parent, root);
          x = x_parent->parent;
          x_parent = x->parent;
        }
        else {
          avltree_rotate_right(x_parent, root);
          x = x_parent->parent;
          x_parent = x->parent;
        }
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
        AVLNode* a = x_parent->right;
        if (a->bal_factor == -1) {
          avltree_rotate_right_left(x_parent, root);
          x = x_parent->parent;
          x_parent = x->parent;
        }
        else {
          avltree_rotate_left(x_parent, root);
          x = x_parent->parent;
          x_parent = x->parent;
        }
        if (x->bal_factor == -1)
          return y;
      }
      break;
    }
  }

  return y;
}

class AVLTree final : private UnCopyable {
public:
  using ValueType = int;
  using SizeType  = std::size_t;
  using Iter      = AVLNodeIter<int, int&, int*>;
  using ConstIter = AVLNodeIter<int, const int&, const int*>;
  using Ref       = int&;
  using ConstRef  = const int&;
private:
  using NodeType  = AVLNode;
  using Link      = NodeType*;
  using ConstLink = const NodeType*;

  SizeType size_{};
  NodeType head_{};

  inline void initialize() noexcept {
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.bal_factor = -2;
  }

  inline Link _get_head() noexcept { return head_.parent; }
  inline ConstLink _get_head() const noexcept { return head_.parent; }
  inline Link _get_tail() noexcept { return &head_; }
  inline ConstLink _get_tail() const noexcept { return &head_; }

  void _tear_from(NodeType* x) noexcept {
    while (x != nullptr) {
      _tear_from(x->right);
      auto* y = x->left;
      destroy_node(x);
      x = y;
    }
  }

  inline NodeType* create_node(const ValueType& v) noexcept {
    NodeType* node = new NodeType();
    node->value = v;
    return node;
  }

  inline void destroy_node(NodeType* node) noexcept {
    delete node;
  }

  inline void insert_aux(NodeType* n) noexcept {
    Link x = _get_head();
    Link y = _get_tail();
    while (x != nullptr) {
      y = x;
      x = n->value < x->value ? x->left : x->right;
    }

    bool insert_left = x != nullptr || y == _get_tail() || n->value < y->value;
    avltree_insert_rebalance(insert_left, n, y, head_);
    ++size_;
  }

  inline void erase_aux(NodeType* node) noexcept {
    NodeType* n = avltree_erase_rebalance(node, head_);
    destroy_node(n);
    --size_;
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
  inline ConstIter end() const noexcept { return ConstIter((AVLNode*)&head_); }
  inline Ref get_head() noexcept { return *begin(); }
  inline ConstRef get_head() const noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(--end()); }
  inline ConstRef get_tail() const noexcept { return *(--end()); }

  inline void clear() noexcept {
    _tear_from(_get_head());
    initialize();
  }

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }

  inline void insert(const ValueType& v) { insert_aux(create_node(v)); }

  inline void erase(Iter pos) noexcept { erase_aux(pos.node); }
  inline void erase(ConstIter pos) noexcept { erase_aux(pos.node); }
};

}

void test_avl3() {
  avl3::AVLTree avl;

  auto show_avl = [&avl] {
    std::cout << "\navltree -> size: "
      << avl.size() << " | empty: " << avl.empty() << std::endl;

    for (auto i = avl.begin(); i != avl.end(); ++i)
      std::cout << "avltree node item: " << *i << std::endl;
  };

  avl.insert(1);
  avl.insert(33);
  avl.insert(22);
  avl.insert(6);
  avl.insert(3);
  show_avl();

  avl.erase(avl.begin());
  show_avl();
}
