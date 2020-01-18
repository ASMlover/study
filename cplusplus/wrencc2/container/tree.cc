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
#include "tree.hh"

namespace wrencc::tree {

inline BasePtr minimum(BasePtr x) noexcept {
  while (x->left != nullptr)
    x = x->left;
  return x;
}

inline BasePtr maximum(BasePtr x) noexcept {
  while (x->right != nullptr)
    x = x->right;
  return x;
}

inline void transplant(BasePtr u, BasePtr v, BasePtr& root) noexcept {
  if (root == u)
    root = v;
  else if (u->parent->left == u)
    u->parent->left = v;
  else
    u->parent->right = v;
}

inline BasePtr left_rotate(BasePtr x, BasePtr& root) noexcept {
  //          |                   |
  //          x                   y
  //           \                 / \
  //            y               x   b
  //           / \               \
  //          a   b               a

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
  //          |                   |
  //          x                   y
  //         /                   / \
  //        y                   a   x
  //       / \                     /
  //      a   b                   b

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

    AVLNodeBase* b = a->right->as_avl();
    if (b->lheight() > b->rheight()) {
      b = right_rotate(b, root)->as_avl();
      b->right->as_avl()->update_height();
      b->update_height();
    }
    a = left_rotate(a, root)->as_avl();
    a->left->as_avl()->update_height();
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

    AVLNodeBase* b = a->left->as_avl();
    if (b->lheight() < b->rheight()) {
      b = left_rotate(b, root)->as_avl();
      b->left->as_avl()->update_height();
      b->update_height();
    }
    a = right_rotate(a, root)->as_avl();
    a->right->as_avl()->update_height();
    a->update_height();

    return a;
  }

  void avl_fixup(BasePtr x, BasePtr& root) noexcept {
    AVLNodeBase* node = x->as_avl();
    while (node != root) {
      int lh = node->lheight();
      int rh = node->rheight();
      int height = std::max(lh, rh) + 1;
      int diff = lh - rh;

      if (node->height != height)
        node->set_height(height);
      else if (diff >= -1 && diff <= 1)
        break;

      if (diff <= -2)
        node = left_fixup(node, root);
      else if (diff >= 2)
        node = right_fixup(node, root);

      node = node->parent->as_avl();
    }
  }

  void insert(
      bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept {
    BasePtr& root = header.parent;

    x->parent = p;
    x->left = x->right = nullptr;
    x->as_avl()->set_height(1);

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
    avl_fixup(x, root);
  }

  void erase(BasePtr x, NodeBase& header) noexcept {
    BasePtr& root = header.parent;
    BasePtr& lmost = header.left;
    BasePtr& rmost = header.right;
    BasePtr z = x;

    BasePtr y = nullptr;
    BasePtr p = nullptr;
    if (x->left != nullptr && x->right != nullptr) {
      x = minimum(x->right);
      y = x->right;
      p = x->parent;
      if (y != nullptr)
        y->parent = p;
      transplant(x, y, root);
      if (x->parent == z)
        p = x;
      x->parent = z->parent;
      x->left = z->left;
      x->right = z->right;
      x->as_avl()->set_height(z->as_avl()->height);
      transplant(z, x, root);
      x->left->parent = x;
      if (x->right != nullptr)
        x->right->parent = x;
    }
    else {
      y = x->left == nullptr ? x->right : x->left;
      p = x->parent;
      transplant(x, y, root);
      if (y != nullptr)
        y->parent = p;
    }
    if (p != nullptr)
      avl_fixup(p, root);

    if (root == z)
      root = y;
    if (lmost == z)
      lmost = z->right == nullptr ? z->parent : minimum(y);
    if (rmost == z)
      rmost = z->left == nullptr ? z->parent : maximum(y);
  }
}

namespace impl::rb {
  inline void insert_fixup(BasePtr x, BasePtr& root) noexcept {
    while (x != root && x->parent->as_rb()->is_red()) {
      if (x->parent == x->parent->parent->left) {
        RBNodeBase* y = x->parent->parent->right->as_rb();
        if (y != nullptr && y->is_red()) {
          x->parent->as_rb()->as_blk();
          y->as_blk();
          x->parent->parent->as_rb()->as_red();
          x = x->parent->parent;
        }
        else {
          if (x == x->parent->right) {
            x = x->parent;
            left_rotate(x, root);
          }
          x->parent->as_rb()->as_blk();
          x->parent->parent->as_rb()->as_red();
          right_rotate(x->parent->parent, root);
        }
      }
      else {
        RBNodeBase* y = x->parent->parent->left->as_rb();
        if (y != nullptr && y->is_red()) {
          x->parent->as_rb()->as_blk();
          y->as_blk();
          x->parent->parent->as_rb()->as_red();
          x = x->parent->parent;
        }
        else {
          if (x == x->parent->left) {
            x = x->parent;
            right_rotate(x, root);
          }
          x->parent->as_rb()->as_blk();
          x->parent->parent->as_rb()->as_red();
          left_rotate(x->parent->parent, root);
        }
      }
    }
    root->as_rb()->as_blk();
  }

  inline void erase_fixup(BasePtr x, BasePtr x_parent, BasePtr& root) noexcept {
    while (x != root && (x == nullptr || x->as_rb()->is_blk())) {
      if (x == x_parent->left) {
        RBNodeBase* w = x_parent->right->as_rb();
        if (w->is_red()) {
          w->as_blk();
          x_parent->as_rb()->as_red();
          left_rotate(x_parent, root);
          w = x_parent->right->as_rb();
        }

        if ((w->left == nullptr || w->left->as_rb()->is_blk())
            && (w->right == nullptr || w->right->as_rb()->is_blk())) {
          w->as_red();
          x = x_parent;
          x_parent = x_parent->parent;
        }
        else {
          if (w->right == nullptr || w->right->as_rb()->is_blk()) {
            if (w->left != nullptr)
              w->left->as_rb()->as_blk();
            w->as_red();
            right_rotate(w, root);
            w = x_parent->right->as_rb();
          }
          w->set_color(x_parent->as_rb()->color);
          x_parent->as_rb()->as_blk();
          if (w->right != nullptr)
            w->right->as_rb()->as_blk();
          left_rotate(x_parent, root);
          break;
        }
      }
      else {
        RBNodeBase* w = x_parent->left->as_rb();
        if (w->is_red()) {
          w->as_blk();
          x_parent->as_rb()->as_red();
          right_rotate(x_parent, root);
          w = x_parent->left->as_rb();
        }

        if ((w->left == nullptr || w->left->as_rb()->is_blk())
            && (w->right == nullptr || w->right->as_rb()->is_blk())) {
          w->as_red();
          x = x_parent;
          x_parent = x_parent->parent;
        }
        else {
          if (w->left == nullptr || w->left->as_rb()->is_blk()) {
            if (w->right != nullptr)
              w->right->as_rb()->as_blk();
            w->as_red();
            left_rotate(w, root);
            w = x_parent->left->as_rb();
          }
          w->set_color(x_parent->as_rb()->color);
          x_parent->as_rb()->as_blk();
          if (w->left != nullptr)
            w->left->as_rb()->as_blk();
          right_rotate(x_parent, root);
          break;
        }
      }
    }
    if (x != nullptr)
      x->as_rb()->as_blk();
  }

  void insert(
      bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept {
    BasePtr& root = header.parent;

    x->parent = p;
    x->left = x->right = nullptr;
    x->as_rb()->as_red();

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

    insert_fixup(x, root);
  }

  void erase(BasePtr z, NodeBase& header) noexcept {
    BasePtr& root = header.parent;
    BasePtr& lmost = header.left;
    BasePtr& rmost = header.right;

    BasePtr x = nullptr;
    BasePtr x_parent = nullptr;
    BasePtr y = z;
    if (y->left != nullptr && y->right != nullptr) {
      y = minimum(y->right);
      x = y->right;
    }
    else {
      x = y->left == nullptr ? y->right : y->left;
    }

    if (y != z) {
      y->left = z->left;
      y->left->parent = y;
      if (y != z->right) {
        x_parent = y->parent;
        if (x != nullptr)
          x->parent = y->parent;
        y->parent->left = x;
        y->right = z->right;
        y->right->parent = y;
      }
      else {
        x_parent = y;
      }

      transplant(z, y, root);
      y->parent = z->parent;
      std::swap(y->as_rb()->color, z->as_rb()->color);
      y = z;
    }
    else {
      x_parent = y->parent;
      if (x != nullptr)
        x->parent = y->parent;
      transplant(z, x, root);

      if (lmost == z)
        lmost = z->right == nullptr ? z->parent : minimum(x);
      if (rmost == z)
        rmost = z->left == nullptr ? z->parent : maximum(x);
    }

    if (y->as_rb()->is_blk())
      erase_fixup(x, x_parent, root);
  }
}

}
