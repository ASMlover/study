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
#pragma once

struct __rbtree_node {
  __rbtree_node* left{};
  __rbtree_node* right{};
  __rbtree_node* parent{};
  int value{};
  bool red{};

  __rbtree_node(bool r = false) : red(r) {}
  __rbtree_node(int v, bool r = false) : value(v), red(r) {}
};

class rbtree {
  __rbtree_node* root_{};
  __rbtree_node* sentinel_{};

  rbtree(const rbtree&) = delete;
  rbtree& operator=(const rbtree&) = delete;

  void destroy(__rbtree_node* node);
  __rbtree_node* get_grandparent(__rbtree_node* node);
  __rbtree_node* get_uncle(__rbtree_node* node);
  __rbtree_node* get_sibling(__rbtree_node* node);
  void insert_fixup(__rbtree_node* node);
  void remove_fixup(__rbtree_node* node);
  void rotate_left(__rbtree_node* node);
  void rotate_right(__rbtree_node* node);
public:
  rbtree(void);
  ~rbtree(void);
  int insert(int v);
  int remove(int v);
  bool find(int v);
};
