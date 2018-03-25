// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "rbtree.h"

rbtree::rbtree(void) {
  sentinel_ = new __rbtree_node();
  root_ = sentinel_;
}

rbtree::~rbtree(void) {
  destroy(root_);
  if (sentinel_ != nullptr)
    delete sentinel_;
}

int rbtree::insert(int v) {
  __rbtree_node* temp = root_;
  __rbtree_node* parent = nullptr;
  auto* node = new __rbtree_node(v, true);
  node->left = node->right = sentinel_;
  while (temp != sentinel_) {
    parent = temp;
    if (temp->value == v) {
      delete node;
      return 0;
    }
    temp = temp->value > v ? temp->right : temp->left;
  }
  if (root_ == sentinel_) {
    root_ = node;
    root_->parent = nullptr;
  }
  else {
    node->parent = parent;
    if (parent->value > v)
      parent->left = node;
    else
      parent->right = node;
  }
  insert_fixup(node);

  return 0;
}

int rbtree::remove(int v) {
  if (root_ == nullptr)
    return 0;

  auto* node = root_;
  while (node != sentinel_) {
    if (node->value > v)
      node = node->left;
    else if (node->value < v)
      node = node->right;
    else
      break;
  }
  if (node == sentinel_)
    return 0;

  __rbtree_node* sub{};
  if (node->left == sentinel_ && node->right == sentinel_) {
    sub = node;
  }
  else if (node->left == sentinel_) {
    sub = node->right;
  }
  else if (node->right == sentinel_) {
    sub = node->left;
  }
  else {
    sub = node->left;
    while (sub->right != sentinel_)
      sub = sub->right;
  }

  if (node != sub)
    node->value = sub->value;
  auto* sub_child = sub->right != sentinel_ ? sub->right : sub->left;
  if (sub->parent != nullptr) {
    if (sub == sub->parent->left)
      sub->parent->left = sub_child;
    else
      sub->parent->right = sub_child;
  }
  else {
    root_ = sub_child;
  }

  sub_child->parent = sub->parent;
  if (!sub->red)
    remove_fixup(sub_child);
  if (sub)
    delete sub;
  sentinel_->parent = nullptr;

  return 0;
}

bool rbtree::find(int v) {
  if (root_ == nullptr)
    return false;

  auto* node = root_;
  while (node != sentinel_) {
    if (node->value < v)
      node = node->right;
    else if (node->value > v)
      node = node->left;
    else
      break;
  }

  return node != sentinel_;
}

void rbtree::destroy(__rbtree_node* node) {
  if (node != nullptr && node != sentinel_) {
    destroy(node->left);
    destroy(node->right);
    delete node;
  }
}

__rbtree_node* rbtree::get_grandparent(__rbtree_node* node) {
  if (node != nullptr && node->parent)
    return node->parent->parent;
  return nullptr;
}

__rbtree_node* rbtree::get_uncle(__rbtree_node* node) {
  auto* grandparent = get_grandparent(node);
  if (grandparent == nullptr)
    return nullptr;
  if (node->parent == grandparent->left)
    return grandparent->right;
  else
    return grandparent->left;
}

__rbtree_node* rbtree::get_sibling(__rbtree_node* node) {
  if (node == nullptr || node->parent == nullptr)
    return nullptr;
  if (node == node->parent->left)
    return node->parent->right;
  else
    return node->parent->left;
}

void rbtree::insert_fixup(__rbtree_node* node) {
  if (node == nullptr)
    return;

  __rbtree_node* uncle = sentinel_;
  __rbtree_node* grandparent{};
  while (node != root_ && node->parent->red) {
    uncle = get_uncle(node);
    grandparent = get_grandparent(node);
    if (uncle != sentinel_ && uncle->red) {
      node->parent->red = false;
      uncle->red = false;
      grandparent->red = true;
      node = grandparent;
    }
    else {
      if (node->parent == grandparent->left) {
        if (node == node->parent->right) {
          node = node->parent;
          rotate_left(node);
        }
        node->parent->red = false;
        grandparent->red = true;
        rotate_right(grandparent);
      }
      else {
        if (node == node->parent->left) {
          node = node->parent;
          rotate_right(node);
        }
        node->parent->red = false;
        grandparent->red = true;
        rotate_left(grandparent);
      }
    }
  }
  root_->red = false;
}

void rbtree::remove_fixup(__rbtree_node* node) {
  __rbtree_node* sibling{};
  while (node != root_ && !node->red) {
    sibling = get_sibling(node);
    if (node == node->parent->left) {
      if (sibling->red) {
        node->parent->red = true;
        sibling->red = false;
        rotate_left(node->parent);
        sibling = node->parent->right;
      }

      if (!sibling->left->red && !sibling->right->red) {
        sibling->red = true;
        node = node->parent;
      }
      else {
        if (!sibling->right->red) {
          sibling->red = true;
          sibling->left->red = false;
          rotate_right(sibling);
          sibling = node->parent->right;
        }
        sibling->red = node->parent->red;
        node->parent->red = false;
        sibling->right->red = false;
        rotate_left(node->parent);
        break;
      }
    }
    else {
      if (sibling->red) {
        node->parent->red = true;
        sibling->red = false;
        rotate_right(node->parent);
        sibling = node->parent->left;
      }

      if (!sibling->left->red && !sibling->right->red) {
        sibling->red = true;
        node = node->parent;
      }
      else {
        if (!sibling->left->red) {
          sibling->red = true;
          sibling->right->red = false;
          rotate_left(sibling);
          sibling = node->parent->left;
        }
        sibling->red = node->parent->red;
        node->parent->red = false;
        sibling->left->red = false;
        rotate_right(node->parent);
        break;
      }
    }
  }
  node->red = false;
}

void rbtree::rotate_left(__rbtree_node* node) {
  if (node == nullptr || node->right == nullptr)
    return;

  auto* rchild = node->right;
  node->right = rchild->left;
  if (rchild->left != nullptr)
    rchild->left->parent = node;
  if (node == root_) {
    root_ = rchild;
    rchild->parent = nullptr;
  }
  else {
    rchild->parent = node->parent;
    if (node == node->parent->left)
      node->parent->left = rchild;
    else
      node->parent->right = rchild;
  }
  rchild->left = node;
  node->parent = rchild;
}

void rbtree::rotate_right(__rbtree_node* node) {
  if (node == nullptr || node->left == nullptr)
    return;

  auto* lchild = node->left;
  node->left = lchild->right;
  if (lchild->right != nullptr)
    lchild->right->parent = node;
  if (node == root_) {
    root_ = lchild;
    lchild->parent = nullptr;
  }
  else {
    lchild->parent = node->parent;
    if (node == node->parent->left)
      node->parent->left = lchild;
    else
      node->parent->right = lchild;
  }
  lchild->right = node;
  node->parent = lchild;
}
