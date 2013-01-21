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
  element_t e;
} *btree_node_t;

struct btree_s {
  int size;
  btree_node_t root;
};

#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define ASSERT_BT(bt) {\
  if (NULL == (bt)) {\
    fprintf(stderr, "btree invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_BT_NODE(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "create btree node failed ...");\
    exit(0);\
  }\
}
#define ASSERT_BT_VISIT(bt, v) {\
  if (NULL == (bt) || NULL == (v)) {\
    fprintf(stderr, "btree arguments invalid ...");\
    exit(0);\
  }\
}


static btree_node_t 
btree_node_create(element_t e) 
{
  btree_node_t new_node = (btree_node_t)malloc(sizeof(*new_node));
  ASSERT_BT_NODE(new_node);

  new_node->lchild = NULL;
  new_node->rchild = NULL;
  new_node->e = e;

  return new_node;
}

static void 
btree_node_delete(btree_t bt, btree_node_t n) 
{
  if (NULL != n) {
    btree_node_delete(bt, n->lchild);
    btree_node_delete(bt, n->rchild);
    free(n);
    --bt->size;
  }
}

static void 
btree_node_preorder(btree_node_t n, void (*visit)(element_t)) 
{
  if (NULL != n) {
    visit(n->e);
    btree_node_preorder(n->lchild, visit);
    btree_node_preorder(n->rchild, visit);
  }
}

static void 
btree_node_inorder(btree_node_t n, void (*visit)(element_t))
{
  if (NULL != n) {
    btree_node_inorder(n->lchild, visit);
    visit(n->e);
    btree_node_inorder(n->rchild, visit);
  }
}

static void 
btree_node_postorder(btree_node_t n, void (*visit)(element_t)) 
{
  if (NULL != n) {
    btree_node_postorder(n->lchild, visit);
    btree_node_postorder(n->rchild, visit);
    visit(n->e);
  }
}

static int 
btree_node(btree_node_t n) 
{
  if (NULL == n)
    return 0;
  else
    return (1 + btree_node(n->lchild) + btree_node(n->rchild));
}

static int 
btree_leave(btree_node_t n)
{
  int leaves = 0;

  if (NULL == n)
    return 0;
  else {
    if (NULL == n->lchild && NULL == n->rchild)
      ++leaves;
    leaves += btree_leave(n->lchild);
    leaves += btree_leave(n->rchild);
  }

  return leaves;
}

static int 
btree_height(btree_node_t n) 
{
  if (NULL == n)
    return 0;
  else 
    return 1 + MAX(btree_height(n->lchild), btree_height(n->rchild));
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
  if (NULL != *bt) {
    btree_clear(*bt);
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
btree_insert(btree_t bt, element_t e)
{
  btree_node_t new_node = btree_node_create(e);
  ASSERT_BT(bt);

  if (NULL == bt->root)
    bt->root = new_node;
  else {
    btree_node_t iter = bt->root;

    while (NULL != iter) {
      if (NULL == iter->lchild || NULL == iter->rchild)
        break;
      else 
        iter = iter->lchild;
    }
    if (NULL == iter->lchild)
      iter->lchild = new_node;
    else 
      iter->rchild = new_node;
  }
  ++bt->size;
}

void 
btree_preorder(btree_t bt, void (*visit)(element_t)) 
{
  ASSERT_BT_VISIT(bt, visit);

  btree_node_preorder(bt->root, visit);
}

void 
btree_inorder(btree_t bt, void (*visit)(element_t)) 
{
  ASSERT_BT_VISIT(bt, visit);

  btree_node_inorder(bt->root, visit);
}

void 
btree_postorder(btree_t bt, void (*visit)(element_t))
{
  ASSERT_BT_VISIT(bt, visit);

  btree_node_postorder(bt->root, visit);
}

int 
btree_nodes(btree_t bt) 
{
  ASSERT_BT(bt);

  return btree_node(bt->root);
}

int 
btree_leaves(btree_t bt) 
{
  ASSERT_BT(bt);

  return btree_leave(bt->root);
}

int 
btree_heights(btree_t bt) 
{
  ASSERT_BT(bt);

  return btree_height(bt->root);
}
