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

typedef struct Node {
  struct Node* prev;
  struct Node* next;
  ElementType value;
} Node;

typedef struct List {
  int length;
  Node node;
} List;

static Node* node_create(ElementType value) {
  Node* node = (Node*)malloc(sizeof(*node));
  if (NULL != node) {
    node->prev = NULL;
    node->next = NULL;
    node->value = value;
  }

  return node;
}

static void list_insert(
    List* list, Node* pos, ElementType value) {
  Node* node = node_create(value);

  node->prev = pos->prev;
  node->next = pos;
  pos->prev->next = node;
  pos->prev = node;

  ++list->length;
}

static void list_remove(List* list, Node* pos) {
  Node* prev;
  Node* next;

  prev = pos->prev;
  next = pos->next;
  prev->next = next;
  next->prev = prev;
  free(pos);

  --list->length;
}

static void list_clear(List* list) {
  assert(NULL != list);

  {
    Node* iter = list->node.next;
    Node* node;
    while (iter != &list->node) {
      node = iter;
      iter = iter->next;
      list_remove(list, node);
    }
    list->length = 0;
  }
}

static List* list_init(void) {
  List* list = (List*)malloc(sizeof(*list));

  if (NULL != list) {
    list->length = 0;
    list->node.prev = &list->node;
    list->node.next = &list->node;
  }

  return list;
}

static void list_destroy(List** list) {
  assert(NULL != *list);

  list_clear(*list);
  free(*list);
  *list = NULL;
}

static int list_length(List* list) {
  assert(NULL != list);

  return list->length;
}

static void list_push_back(List* list, ElementType value) {
  assert(NULL != list);

  list_insert(list, &list->node, value);
}

static void list_push_front(List* list, ElementType value) {
  assert(NULL != list);

  list_insert(list, list->node.next, value);
}

static void list_pop_back(List* list) {
  assert(NULL != list);

  {
    Node* prev = (&list->node)->prev;
    if (prev != &list->node)
      list_remove(list, prev);
  }
}

static void list_pop_front(List* list) {
  assert(NULL != list);

  list_remove(list, list->node.next);
}

static void list_display(List* list) {
  assert(NULL != list);

  {
    Node* iter = list->node.next;
    while (iter != &list->node) {
      fprintf(stdout, "DoubleList element value is : %d\n", iter->value);
      iter = iter->next;
    }
    fprintf(stdout, "\n");
  }
}

void double_list(void) {
  List* list = list_init();
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
