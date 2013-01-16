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
#include "vlist.h"


typedef struct vlist_node_s {
  struct vlist_node_s* next;
  element_t* elements;
} *vlist_node_t;

struct vlist_s {
  vlist_node_t head;
  int size;
  int offset;
};


#define ASSERT_VLIST(vl) {\
  if (NULL == (vl)) {\
    fprintf(stderr, "vlist invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_VLIST_NODE(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "create vlist node failed ...");\
    exit(0);\
  }\
}
#define ASSERT_VLIST_UNDERFLOW(vl) {\
  if (1 == (vl)->size && 0 == (vl)->offset) {\
    fprintf(stderr, "vlist underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_VLIST_CHECK(i) {\
  if ((i) < 0) {\
    fprintf(stderr, "index out of range ...");\
    exit(0);\
  }\
}



static vlist_node_t 
vlist_node_create(int elem_sz) 
{
  int malloc_len = sizeof(struct vlist_node_s) 
    + sizeof(element_t) * elem_sz;
  vlist_node_t new_node = (vlist_node_t)malloc(malloc_len);
  ASSERT_VLIST_NODE(new_node);

  new_node->next = NULL;
  new_node->elements = (element_t*)(new_node + 1);

  return new_node;
}

static void 
vlist_clear(vlist_t vl)
{
  vlist_node_t iter;
  while (NULL != vl->head) {
    iter = vl->head;
    vl->head = vl->head->next;
    free(iter);
  }
  vl->size = 1;
  vl->offset = 0;
}




vlist_t 
vlist_create(void) 
{
  vlist_t vl = (vlist_t)malloc(sizeof(*vl));
  ASSERT_VLIST(vl);

  vl->head = vlist_node_create(1);
  vl->size = 1;
  vl->offset = 0;

  return vl;
}

void 
vlist_delete(vlist_t* vl)
{
  if (NULL != *vl) {
    vlist_clear(*vl);
    free(*vl);
    *vl = NULL;
  }
}

int 
vlist_empty(vlist_t vl) 
{
  ASSERT_VLIST(vl);

  return (1 == vl->size && 0 == vl->offset);
}

int 
vlist_size(vlist_t vl) 
{
  ASSERT_VLIST(vl);

  return (2 * vl->size - vl->offset - 2);
}

void 
vlist_insert(vlist_t vl, element_t e)
{
  ASSERT_VLIST(vl);

  if (0 == vl->offset) {
    vlist_node_t new_node = vlist_node_create(vl->size << 1);
    vl->offset = (vl->size <<= 1);
    new_node->next = vl->head;
    vl->head = new_node;
  }

  *(vl->head->elements + (--vl->offset)) = e;
}

element_t 
vlist_remove(vlist_t vl) 
{
  element_t e;
  ASSERT_VLIST(vl);
  ASSERT_VLIST_UNDERFLOW(vl);

  e = vl->head->elements[vl->offset++];
  if (vl->offset == vl->size) {
    vl->offset = 0;
    if (vl->size > 1) {
      vlist_node_t old_node = vl->head;
      vl->head = vl->head->next;
      vl->size >>= 1;
      free(old_node);
    }
  }

  return e;
}

element_t 
vlist_get(vlist_t vl, int i)
{
  vlist_node_t iter;
  int top;
  ASSERT_VLIST(vl);
  ASSERT_VLIST_CHECK(i);

  iter = vl->head;
  top  = vl->size;
  i    = i + vl->offset;
  if (i + 2 >= (top << 1)) {
    fprintf(stderr, "out of range ...");
    exit(0);
  }

  while (NULL != iter && i >= top) {
    iter = iter->next;
    i ^= top;
    top >>= 1;
  }

  return *(iter->elements + i);
}
