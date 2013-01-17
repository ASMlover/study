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
#include "skip_list.h"


typedef struct skip_list_node_s {
  int key;
  void* val;
  struct skip_list_node_s* forward[1];
} *skip_list_node_t;

struct skip_list_s {
  int size;
  int level;
  skip_list_node_t head;
};

#define NUM_OF_LEVELS   (16)
#define MAX_LEVEL       (15)
#define ASSERT_SL(sl) {\
  if (NULL == (sl)) {\
    fprintf(stderr, "skip list invalid ...");\
    exit(0);\
  }\
}

static void 
SHOW_DEBUG(const char* s, void* address) 
{
#ifndef NDEBUG
  fprintf(stdout, "%s : %p\n", s, address);
#endif 
}



static skip_list_node_t 
skip_list_node_create(int level)
{
  int size = sizeof(struct skip_list_node_s) 
    + level * sizeof(skip_list_node_t);
  skip_list_node_t new_node = (skip_list_node_t)malloc(size);
  if (NULL == new_node) {
    fprintf(stderr, "create skip list node failed ...");
    exit(0);
  }

  return new_node;
}

static void 
skip_list_clear(skip_list_t sl) 
{
  int i;
  skip_list_node_t node;
  skip_list_node_t iter = sl->head;

  while (sl->head != iter->forward[0]) {
      node = iter->forward[0];
      iter = iter->forward[0];
      SHOW_DEBUG("free skip list node", node);
      free(node);
  }
  for (i = 0; i < NUM_OF_LEVELS; ++i)
    sl->head->forward[i] = sl->head;
  sl->level = 0;
  sl->size  = 0;
}




skip_list_t 
skip_list_create(void) 
{
  int i;
  skip_list_t sl = (skip_list_t)malloc(sizeof(*sl));
  ASSERT_SL(sl);

  sl->size  = 0;
  sl->level = 0;
  sl->head  = skip_list_node_create(NUM_OF_LEVELS);
  for (i = 0; i < NUM_OF_LEVELS; ++i) 
    sl->head->forward[i] = sl->head;

  return sl;
}

void 
skip_list_delete(skip_list_t* sl) 
{
  if (NULL != *sl) {
    skip_list_clear(*sl);
    free(*sl);
    *sl = NULL;
  }
}

int 
skip_list_empty(skip_list_t sl) 
{
  ASSERT_SL(sl);

  return (0 == sl->size);
}

int 
skip_list_size(skip_list_t sl) 
{
  ASSERT_SL(sl);

  return (sl->size);
}

void 
skip_list_insert(skip_list_t sl, int key, element_t e)
{
  int i, new_level;
  skip_list_node_t update_node[NUM_OF_LEVELS];
  skip_list_node_t iter;
  ASSERT_SL(sl);

  iter = sl->head;
  for (i = sl->level; i >= 0; --i) {
    while (sl->head != iter->forward[i] && iter->forward[i]->key < key)
      iter = iter->forward[i];
    update_node[i] = iter;
  }
  iter = iter->forward[0];

  if (sl->head != iter && key == iter->key) {
    fprintf(stderr, "this key node has been inserted ...");
    exit(0);
  }

  for (new_level = 0; 
    rand() < RAND_MAX / 2 && new_level < MAX_LEVEL; ++new_level) {
  }

  if (new_level > sl->level) {
    for (i = sl->level + 1; i <= new_level; ++i) 
      update_node[i] = sl->head;
    sl->level = new_level;
  }

  iter = skip_list_node_create(new_level);
  SHOW_DEBUG("allocate skip list node", iter);
  iter->key = key;
  iter->val = e;
  ++sl->size;

  for (i = 0; i <= new_level; ++i) {
    iter->forward[i] = update_node[i]->forward[i];
    update_node[i]->forward[i] = iter;
  }
}

element_t 
skip_list_remove(skip_list_t sl, int key) 
{
  int i;
  element_t e;
  skip_list_node_t update_node[NUM_OF_LEVELS];
  skip_list_node_t iter;
  ASSERT_SL(sl);

  iter = sl->head;
  for (i = sl->level; i >= 0; --i) {
    while (sl->head != iter->forward[i] && iter->forward[i]->key < key)
      iter = iter->forward[i];
    update_node[i] = iter;
  }
  iter = iter->forward[0];

  if (iter == sl->head || iter->key != key) {
    fprintf(stderr, "key not found ...");
    exit(0);
  }

  for (i = 0; i <= sl->level; ++i) {
    if (iter != update_node[i]->forward[i])
      break;
    update_node[i]->forward[i] = iter->forward[i];
  }
  e = iter->val;
  SHOW_DEBUG("free skip list node", iter);
  free(iter);
  --sl->size;

  while (sl->level > 0 && (sl->head->forward[sl->level] == sl->head))
    --sl->level;

  return e;
}
