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

struct AVLNode {
  AVLNode* lchild{};
  AVLNode* rchild{};
  int value{};
  int height{0};

  AVLNode(int v) noexcept : value(v) {}
};

class AVLTree final : private UnCopyable {
  AVLNode* root_{};
public:
  AVLTree() noexcept {}
  ~AVLTree() noexcept {}

  int get_height(AVLNode* node) { return node == nullptr ? 0 : node->height; }

  AVLNode* ll_rotate(AVLNode* root) {
    // LL
    AVLNode* lchild = root->lchild;
    root->lchild = lchild->rchild;
    lchild->rchild = root;

    root->height = std::max(get_height(root->lchild), get_height(root->rchild)) + 1;
    lchild->height = std::max(get_height(lchild->lchild), root->height) + 1;

    return lchild;
  }

  AVLNode* rr_rotate(AVLNode* root) {
    // RR
    AVLNode* rchild = root->rchild;
    root->rchild = rchild->lchild;
    rchild->lchild = root;

    root->height = std::max(get_height(root->lchild), get_height(root->rchild)) + 1;
    rchild->height = std::max(get_height(rchild->rchild), root->height) + 1;

    return rchild;
  }

  AVLNode* lr_rotate(AVLNode* root) {
    // L-R
    root->lchild = rr_rotate(root->lchild);
    return ll_rotate(root);
  }

  AVLNode* rl_rotate(AVLNode* root) {
    // R-L
    root->rchild = ll_rotate(root->rchild);
    return rr_rotate(root);
  }

  AVLNode* insert(AVLNode*& root, int value) {
    if (root == nullptr) {
      root = new AVLNode(value);
    }
    else if (value < root->value) {
      root->lchild = insert(root->lchild, value);
      if (get_height(root->lchild) - get_height(root->rchild) == 2) {
        if (value < root->lchild->value)
          root = ll_rotate(root);
        else
          root = lr_rotate(root);
      }
    }
    else if (value > root->value) {
      root->rchild = insert(root->rchild, value);
      if (get_height(root->rchild) - get_height(root->lchild) == 2) {
        if (value > root->rchild->value)
          root = rr_rotate(root);
        else
          root = rl_rotate(root);
      }
    }

    root->height = std::max(get_height(root->lchild), get_height(root->rchild)) + 1;
    return root;
  }

  void insert(int value) {
    insert(root_, value);
  }

  void print_prev(AVLNode* node) {
    if (node == nullptr)
      return;

    std::cout << node->value << std::endl;
    print_prev(node->lchild);
    print_prev(node->rchild);
  }

  void print_prev() {
    print_prev(root_);
  }
};

void test_avl() {
  AVLTree avl;

  avl.insert(3);
  avl.insert(2);
  avl.insert(1);
  avl.insert(4);
  avl.insert(5);
  avl.insert(6);
  avl.insert(7);
  avl.insert(16);
  avl.insert(15);
  avl.insert(14);
  avl.insert(13);
  avl.insert(12);
  avl.insert(11);
  avl.insert(10);
  avl.insert(8);
  avl.insert(9);

  avl.print_prev();
}
