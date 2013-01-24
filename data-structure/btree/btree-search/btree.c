/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include "btree.h"


typedef struct btree_node_s {
  struct btree_node_s* lchild;
  struct btree_node_s* rchild;
  int key;
  element_t val;
} *btree_node_t;

struct btree_s {
  int size;
  btree_node_t root;
};

typedef enum found_result_e {
  found_result_no   = 0, 
  found_result_yes  = 1, 
} found_result_t;

#define ASSERT_BT(bt) {\
  if (NULL == (bt)) {\
    fprintf(stderr, "search binary tree invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_BT_NODE(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "create search binary tree node failed ...");\
    exit(0);\
  }\
}
#define ASSERT_BT_UNDERFLOW(bt) {\
  if (NULL == (bt)->root) {\
    fprintf(stderr, "search binary tree underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_BT_ARGUMENTS(bt, v) {\
  if (NULL == (bt) || NULL == (v)) {\
    fprintf(stderr, "search binary tree arguments invalid ...");\
    exit(0);\
  }\
}



static btree_node_t 
btree_node_create(int key, element_t val)
{
  btree_node_t new_node = (btree_node_t)malloc(sizeof(*new_node));
  ASSERT_BT_NODE(new_node);

  new_node->lchild = new_node->rchild = NULL;
  new_node->key = key;
  new_node->val = val;

  return new_node;
}

static void 
btree_node_delete(btree_t bt, btree_node_t node) 
{
  if (NULL != node) {
    btree_node_delete(bt, node->lchild);
    btree_node_delete(bt, node->rchild);
    free(node);
    --bt->size;
  }
}

static void 
btree_node_delete_from_tree(btree_node_t* node, element_t* val)
{
  btree_node_t temp = NULL;

  if (NULL == (*node)->lchild && NULL == (*node)->rchild) {
    *val = (*node)->val;
    free(*node);
    *node = NULL;
  }
  else if (NULL == (*node)->lchild) {
    temp = *node;
    *node = (*node)->rchild;
    *val = temp->val;
    free(temp);
  }
  else if (NULL == (*node)->rchild) {
    temp = *node;
    *node = (*node)->lchild;
    *val = temp->val;
    free(temp);
  }
  else {
    btree_node_t del = (*node)->rchild;
    while (NULL != del->lchild) {
      temp = del;
      del = del->lchild;
    }
    *val = (*node)->val;
    (*node)->key = del->key;
    (*node)->val = del->val;
    if (NULL == temp)
      (*node)->rchild = del->rchild;
    else 
      temp->lchild = del->rchild;
    free(del);
  }
}

static void 
preorder(btree_node_t node, void (*visit)(int, element_t))
{
  if (NULL != node) {
    visit(node->key, node->val);
    preorder(node->lchild, visit);
    preorder(node->rchild, visit);
  }
}

static void 
inorder(btree_node_t node, void (*visit)(int, element_t))
{
  if (NULL != node) {
    inorder(node->lchild, visit);
    visit(node->key, node->val);
    inorder(node->rchild, visit);
  }
}

static void 
postorder(btree_node_t node, void (*visit)(int, element_t)) 
{
  if (NULL != node) {
    postorder(node->lchild, visit);
    postorder(node->rchild, visit);
    visit(node->key, node->val);
  }
}







btree_t 
btree_create(void) 
{
  btree_t bt = (btree_t)malloc(sizeof(*bt));
  ASSERT_BT(bt);

  bt->size = 0;
  bt->root = NULL;

  return bt;
}

void 
btree_delete(btree_t* bt)
{
  if (NULL == *bt) {
    btree_clear(*bt);
    free(*bt);
    *bt = NULL;
  }
}

int 
btree_empty(btree_t bt) 
{
  ASSERT_BT(bt);

  return (NULL == bt->root);
}

int 
btree_size(btree_t bt) 
{
  ASSERT_BT(bt);

  return (bt->size);
}

void 
btree_clear(btree_t bt) 
{
  ASSERT_BT(bt);

  btree_node_delete(bt, bt->root);
  bt->root = NULL;
}

void 
btree_insert(btree_t bt, int key, element_t val) 
{
  btree_node_t new_node, iter, node = NULL;
  ASSERT_BT(bt);

  new_node = btree_node_create(key, val);
  iter = bt->root;
  while (NULL != iter) {
    node = iter;
    if (key < iter->key)
      iter = iter->lchild;
    else 
      iter = iter->rchild;
  }

  if (NULL == node)
    bt->root = new_node;
  else if (key < node->key)
    node->lchild = new_node;
  else 
    node->rchild = new_node;
  ++bt->size;
}

element_t 
btree_remove(btree_t bt, int key) 
{
  element_t val = NULL;
  btree_node_t iter, node = NULL;
  int found = found_result_no;
  ASSERT_BT(bt);
  ASSERT_BT_UNDERFLOW(bt);

  iter = bt->root;
  while (NULL != iter && !found) {
    if (key == iter->key) 
      found = found_result_yes;
    else {
      node = iter;
      if (key < iter->key)
        iter = iter->lchild;
      else 
        iter = iter->rchild;
    }
  }

  if (found) {
    if (bt->root == iter)
      btree_node_delete_from_tree(&bt->root, &val);
    else if (key < node->key) 
      btree_node_delete_from_tree(&node->lchild, &val);
    else 
      btree_node_delete_from_tree(&node->rchild, &val);
    --bt->size;
  }

  return val;
}

element_t 
btree_get(btree_t bt, int key)
{
  element_t val = NULL;
  btree_node_t iter;
  ASSERT_BT(bt);

  iter = bt->root;
  while (NULL != iter) {
    if (key == iter->key)
      val = iter->val;
    else if (key < iter->key)
      iter = iter->lchild;
    else 
      iter = iter->rchild;
  }

  return val;
}

void 
btree_preorder(btree_t bt, void (*visit)(int, element_t))
{
  ASSERT_BT_ARGUMENTS(bt, visit);

  preorder(bt->root, visit);
}

void 
btree_inorder(btree_t bt, void (*visit)(int, element_t)) 
{
  ASSERT_BT_ARGUMENTS(bt, visit);

  inorder(bt->root, visit);
}

void 
btree_postorder(btree_t bt, void (*visit)(int, element_t)) 
{
  ASSERT_BT_ARGUMENTS(bt, visit);

  postorder(bt->root, visit);
}
