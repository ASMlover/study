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
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>
#include "common.hh"

namespace avl2 {

struct AVLNode {
  AVLNode* left{};
  AVLNode* right{};
  AVLNode* parent{};
  int height{};
  int value;

  AVLNode(int v, AVLNode* p = nullptr) noexcept : parent(p), value(v) {}

  inline bool is_empty() const noexcept { return parent == this; }

  inline void set_link(AVLNode* p, AVLNode** avl_link) {
    parent = p;
    height = 0;
    left = right = nullptr;
    avl_link[0] = this;
  }

  inline int left_height() const noexcept {
    return left != nullptr ? left->height : 0;
  }

  inline int right_height() const noexcept {
    return right != nullptr ? right->height : 0;
  }

  void update_height() {
    int hl = left_height();
    int hr = right_height();
    height = std::max(hl, hr) + 1;
  }
};

struct AVLRoot {
  AVLNode* _node{};

  AVLNode* get_first() const {
    AVLNode* node = _node;

    if (node == nullptr)
      return nullptr;
    while (node->left != nullptr)
      node = node->left;
    return node;
  }

  AVLNode* get_last() const {
    AVLNode* node = _node;
    if (node == nullptr)
      return nullptr;
    while (node->right != nullptr)
      node = node->right;
    return node;
  }

  AVLNode* get_next(AVLNode* node) const {
    if (node == nullptr)
      return nullptr;

    if (node->right != nullptr) {
      node = node->right;
      while (node->left != nullptr)
        node = node->left;
    }
    else {
      while (true) {
        AVLNode* last = node;
        node = node->parent;
        if (node == nullptr || node->left == last)
          break;
      }
    }
    return node;
  }

  AVLNode* get_prev(AVLNode* node) {
    if (node == nullptr)
      return nullptr;

    if (node->left != nullptr) {
      node = node->left;
      while (node->right != nullptr)
        node = node->right;
    }
    else {
      while (true) {
        AVLNode* last = node;
        node = node->parent;
        if (node == nullptr || node->right == last)
          break;
      }
    }
    return node;
  }

  void replace_child(AVLNode* old_node, AVLNode* new_node, AVLNode* parent) {
    if (parent) {
      if (parent->left == old_node)
        parent->left = new_node;
      else
        parent->right = new_node;
    }
    else {
      _node = new_node;
    }
  }

  AVLNode* rotate_left(AVLNode* node) {
    AVLNode* right = node->right;
    AVLNode* parent = node->parent;
    node->right = right->left;
    if (right->left != nullptr)
      right->left->parent = node;
    right->left = node;
    right->parent = parent;
    replace_child(node, right, parent);
    node->parent = right;

    return right;
  }

  AVLNode* rotate_right(AVLNode* node) {
    AVLNode* left = node->left;
    AVLNode* parent = node->parent;
    node->left = left->right;
    if (left->right != nullptr)
      left->right->parent = node;
    left->right = node;
    left->parent = parent;
    replace_child(node, left, parent);
    node->parent = left;

    return left;
  }

  void replace_node(AVLNode* victim, AVLNode* new_node) {
    AVLNode* parent = victim->parent;
    replace_child(victim, new_node, parent);
    if (victim->left != nullptr)
      victim->left->parent = new_node;
    if (victim->right != nullptr)
      victim->right->parent = new_node;
    new_node->left = victim->left;
    new_node->right = victim->right;
    new_node->parent = victim->parent;
    new_node->height = victim->height;
  }

  AVLNode* fix_left(AVLNode* node) {
    AVLNode* right = node->right;
    int rh0 = right->left_height();
    int rh1 = right->right_height();
    if (rh0 > rh1) {
      right = rotate_right(right);
      right->right->update_height();
      right->update_height();
    }
    node = rotate_left(node);
    node->left->update_height();
    node->update_height();

    return node;
  }

  AVLNode* fix_right(AVLNode* node) {
    AVLNode* left = node->left;
    int lh0 = left->left_height();
    int lh1 = left->right_height();
    if (lh0 < lh1) {
      left = rotate_left(left);
      left->left->update_height();
      left->update_height();
    }
    node = rotate_right(node);
    node->right->update_height();
    node->update_height();

    return node;
  }

  void rebalance(AVLNode* node) {
    while (node != nullptr) {
      int h0 = node->left_height();
      int h1 = node->right_height();
      int diff = h0 - h1;
      int height = std::max(h0, h1) + 1;
      if (node->height != height)
        node->height = height;
      else if (diff >= -1 && diff <= 1)
        break;

      if (diff <= -2)
        node = fix_left(node);
      else if (diff >= 2)
        node = fix_right(node);
      node = node->parent;
    }
  }

  void insert_post(AVLNode* node) {
    node->height = 1;

    for (node = node->parent; node != nullptr; node = node->parent) {
      int h0 = node->left_height();
      int h1 = node->right_height();
      int diff = h0 - h1;
      int height = std::max(h0, h1) + 1;
      if (node->height == height)
        break;
      node->height = height;

      if (diff <= -2)
        node = fix_left(node);
      else if (diff >= 2)
        node = fix_right(node);
    }
  }

  void erase(AVLNode* node) {
    AVLNode* child;
    AVLNode* parent;
    if (node->left != nullptr && node->right != nullptr) {
      AVLNode* old = node;
      node = node->right;
      AVLNode* left;
      while ((left = node->left) != nullptr)
        node = left;
      child = node->right;
      parent = node->parent;
      if (child != nullptr)
        child->parent = parent;
      replace_child(node, child, parent);
      if (node->parent == old)
        parent = node;
      node->left = old->left;
      node->right = old->right;
      node->parent = old->parent;
      node->height = old->height;
      replace_child(old, node, old->parent);
      old->left->parent = node;
      if (old->right != nullptr)
        old->right->parent = node;
    }
    else {
      if (node->left == nullptr)
        child = node->right;
      else
        child = node->left;
      parent = node->parent;
      replace_child(node, child, parent);
      if (child != nullptr)
        child->parent = parent;
    }

    if (parent != nullptr)
      rebalance(parent);
  }

  AVLNode* tear(AVLNode*& next) {
    AVLNode* node = next;
    AVLNode* parent;
    if (node == nullptr) {
      if (_node == nullptr)
        return nullptr;
      node = _node;
    }
    while (true) {
      if (node->left != nullptr)
        node = node->left;
      else if (node->right != nullptr)
        node = node->right;
      else
        break;
    }

    parent = node->parent;
    if (parent == nullptr) {
      next = nullptr;
      _node = nullptr;
      return node;
    }
    if (parent->left == node)
      parent->left = nullptr;
    else
      parent->right = nullptr;
    node->height = 0;
    next = parent;

    return node;
  }
};

class AVLTree final : private UnCopyable {
  AVLRoot root_;
  std::size_t count_{};

public:
  AVLTree() noexcept { root_._node = nullptr; }

  inline bool is_empty() const noexcept { return count_ == 0; }
  inline std::size_t count() const noexcept { return count_; }
  inline AVLNode* begin() noexcept { return root_.get_first(); }
  inline const AVLNode* begin() const noexcept { return root_.get_first(); }
  inline AVLNode* end() noexcept { return root_.get_last(); }
  inline const AVLNode* end() const noexcept { return root_.get_last(); }

  inline int get_head() const noexcept { return begin()->value; }
  inline int get_tail() const noexcept { return end()->value; }

  void append(int value) {
    AVLNode** link = &root_._node;
    AVLNode* parent = nullptr;
    while (link[0] != nullptr) {
      parent = link[0];
      if (value == parent->value)
        return;
      else if (parent->value < value)
        link = &parent->left;
      else
        link = &parent->right;
    }
    AVLNode* node = new AVLNode(value, parent);
    link[0] = node;
    root_.insert_post(node);
    ++count_;
  }

  void remove(AVLNode* node) {
    if (!node->is_empty()) {
      root_.erase(node);
      node->parent = node;
      --count_;
      delete node;
    }
  }

  void clear() {
    AVLNode* next = nullptr;
    AVLNode* node = nullptr;
    while (true) {
      node = root_.tear(next);
      if (node == nullptr)
        break;
      node->parent = node;
      --count_;
      delete node;
    }
  }
};

}

void test_avl2() {
  avl2::AVLTree avl;
  std::cout << avl.count() << " -> " << avl.is_empty() << std::endl;

  avl.append(34);
  avl.append(44);
  avl.append(23);
  avl.append(5);
  avl.append(55);
  avl.append(9);

  std::cout << avl.count() << " -> " << avl.get_head() << " : " << avl.get_tail() << std::endl;
}
