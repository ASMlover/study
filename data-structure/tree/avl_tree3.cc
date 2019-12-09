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
};

inline AVLNode* avltree_incr(AVLNode* node) noexcept {
  if (node->right != nullptr) {
    node = node->right;
    while (node->left != nullptr)
      node = node->left;
  }
  else {
    AVLNode* parent = node->parent;
    while (node == parent) {
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
    while (node == parent) {
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
  using Ref   = _Tp&;
  using Ptr   = _Tp*;

  AVLNode* node{};

  AVLNodeIter() noexcept {}
  AVLNodeIter(AVLNode* n) noexcept : node(n) {}
  AVLNodeIter(const Iter& x) noexcept : node(x.node) {}

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

class AVLTree final : private UnCopyable {
public:
  using ValueType = int;
  using Iter      = AVLNodeIter<int, int&, int*>;
  using ConstIter = AVLNodeIter<int, const int&, const int*>;
  using Ref       = int&;
  using ConstRef  = const int&;
private:
  using NodeType  = AVLNode;

  std::size_t size_{};
  NodeType head_{};

  inline void initialize() noexcept {
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.bal_factor = -2;
  }
};

}
