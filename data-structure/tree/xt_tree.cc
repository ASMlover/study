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
#include "xt_tree.hh"

namespace xt::tree {

namespace impl {
  inline BasePtr minimum(BasePtr x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  inline ConstBasePtr minimum(ConstBasePtr x) noexcept {
    return minimum(const_cast<BasePtr>(x));
  }

  inline BasePtr maximum(BasePtr x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  inline ConstBasePtr maximum(ConstBasePtr x) noexcept {
    return maximum(const_cast<BasePtr>(x));
  }

  inline void transplant(BasePtr u, BasePtr v, BasePtr& root) noexcept {
    if (u->parent == nullptr)
      root = v;
    else if (u->parent->left == u)
      u->parent->left = v;
    else
      u->parent->right = v;
  }

  inline BasePtr left_rotate(BasePtr x, BasePtr& root) noexcept {
    //            |                   |
    //            x                   y
    //             \                 / \
    //              y               x   b
    //             / \               \
    //            a   b               a

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
    //            |                   |
    //            x                   y
    //           /                   / \
    //          y                   a   x
    //         / \                     /
    //        a   b                   b

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

namespace impl::avl {
  inline AVLNodeBase* left_fixup(AVLNodeBase* a, BasePtr& root) noexcept {
    //            |                   |                   |
    //            a                   a                   c
    //           / \                 / \                 / \
    //         [d]  b              [d]  c               /   \
    //             / \                 / \             a     b
    //            c  [g]              e   b           / \   / \
    //           / \                     / \        [d]  e f  [g]
    //          e   f                   f  [g]

    AVLNodeBase* b = a->right->as<AVLNodeBase>();
    if (b->lheight() > b->rheight()) {
      b = right_rotate(b, root)->as<AVLNodeBase>();
      b->right->as<AVLNodeBase>()->update_height();
      b->update_height();
    }
    a = left_rotate(a, root)->as<AVLNodeBase>();
    a->left->as<AVLNodeBase>()->update_height();
    a->update_height();

    return a;
  }

  inline AVLNodeBase* right_fixup(AVLNodeBase* a, BasePtr& root) noexcept {
    //            |                   |                   |
    //            a                   a                   c
    //           / \                 / \                 / \
    //          b  [g]              c  [g]              /   \
    //         / \                 / \                 b     a
    //       [d]  c               b   f               / \   / \
    //           / \             / \                [d]  e f  [g]
    //          e   f          [d]  e

    AVLNodeBase* b = a->left->as<AVLNodeBase>();
    if (b->lheight() < b->rheight()) {
      b = left_rotate(b, root)->as<AVLNodeBase>();
      b->left->as<AVLNodeBase>()->update_height();
      b->update_height();
    }
    a = right_rotate(a, root)->as<AVLNodeBase>();
    a->right->as<AVLNodeBase>()->update_height();
    a->update_height();

    return a;
  }

  void insert(
      bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept {
  }

  void erase(BasePtr x, NodeBase& header) noexcept {
  }
}

}

#include <iostream>

template <typename Tree> void show_t(Tree& t, const char* s = "tree") {
  std::cout << "\n" << s << " -> size: " << t.size() << std::endl;
  if (!t.empty()) {
    std::cout << s << " -> " << t.get_head() << "|" << t.get_tail() << std::endl;
  }
  for (auto i = t.begin(); i != t.end(); ++i)
    std::cout << s << " -> item value: " << *i << std::endl;
}

void test_tree2() {
  {
    xt::tree::AVLTree<int> t;
    show_t(t, "AVLTree");
  }

  {
    xt::tree::RBTree<int> t;
    show_t(t, "RBTree");
  }
}
