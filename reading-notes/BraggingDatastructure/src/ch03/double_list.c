/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef int ElementType;

typedef struct ListNode {
  struct ListNode* prev;
  struct ListNode* next;
  ElementType value;
} ListNode;

typedef struct DoubleList {
  int length;
  ListNode node;
} DoubleList;

static ListNode* node_create(ElementType value) {
  ListNode* node = (ListNode*)malloc(sizeof(*node));
  if (NULL != node) {
    node->prev = NULL;
    node->next = NULL;
    node->value = value;
  }

  return node;
}

static void list_insert(
    DoubleList* list, ListNode* pos, ElementType value) {
  ListNode* node = node_create(value);

  node->prev = pos->prev;
  node->next = pos;
  pos->prev->next = node;
  pos->prev = node;

  ++list->length;
}

static void list_remove(DoubleList* list, ListNode* pos) {
  ListNode* prev;
  ListNode* next;

  prev = pos->prev;
  next = pos->next;
  prev->next = next;
  next->prev = prev;
  free(pos);

  --list->length;
}

static void list_clear(DoubleList* list) {
  assert(NULL != list);

  {
    ListNode* iter = list->node.next;
    ListNode* node;
    while (iter != &list->node) {
      node = iter;
      iter = iter->next;
      list_remove(list, node);
    }
    list->length = 0;
  }
}

static DoubleList* list_init(void) {
  DoubleList* list = (DoubleList*)malloc(sizeof(*list));

  if (NULL != list) {
    list->length = 0;
    list->node.prev = &list->node;
    list->node.next = &list->node;
  }

  return list;
}

static void list_destroy(DoubleList** list) {
  assert(NULL != *list);

  list_clear(*list);
  free(*list);
  *list = NULL;
}

static int list_length(DoubleList* list) {
  assert(NULL != list);

  return list->length;
}

static void list_push_back(DoubleList* list, ElementType value) {
  assert(NULL != list);

  list_insert(list, &list->node, value);
}

static void list_push_front(DoubleList* list, ElementType value) {
  assert(NULL != list);

  list_insert(list, list->node.next, value);
}

static void list_pop_back(DoubleList* list) {
  assert(NULL != list);

  {
    ListNode* prev = (&list->node)->prev;
    if (prev != &list->node)
      list_remove(list, prev);
  }
}

static void list_pop_front(DoubleList* list) {
  assert(NULL != list);

  list_remove(list, list->node.next);
}

static void list_display(DoubleList* list) {
  assert(NULL != list);

  {
    ListNode* iter = list->node.next;
    while (iter != &list->node) {
      fprintf(stdout, "DoubleList element value is : %d\n", iter->value);
      iter = iter->next;
    }
    fprintf(stdout, "\n");
  }
}

void double_list(void) {
  DoubleList* list = list_init();
  int i;

  fprintf(stdout, "\n\n==========================================\n");

  fprintf(stdout, "DoubleList length = %d\n", list_length(list));

  for (i = 0; i < 10; ++i) 
    list_push_back(list, i * i);
  fprintf(stdout, "DoubleList length = %d\n", list_length(list));
  for (i = 0; i < 10; ++i) 
    list_push_front(list, (i + 100) * 3);
  fprintf(stdout, "DoubleList length = %d\n", list_length(list));
  list_display(list);

  list_pop_back(list);
  list_pop_front(list);
  fprintf(stdout, "DoubleList length = %d\n", list_length(list));
  list_display(list);

  list_clear(list);
  fprintf(stdout, "DoubleList length = %d\n", list_length(list));

  list_destroy(&list);
}
