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
  ElementType value;
  struct Node* next;
} Node;

typedef struct List {
  Node* head;
  int length;
} List;

static Node* node_create(ElementType value) {
  Node* node = (Node*)malloc(sizeof(*node));
  if (NULL == node)
    return NULL;

  node->value = value;
  node->next = NULL;

  return node;
}

static void list_clear(List* list) {
  assert(NULL != list);

  {
    Node* node;
    while (NULL != list->head) {
      node = list->head;
      list->head = node->next;
      free(node);
    }
    list->length = 0;
  }
}

static List* list_init(void) {
  List* list = (List*)malloc(sizeof(*list));
  if (NULL == list)
    return NULL;

  list->head = NULL;
  list->length = 0;

  return list;
}

static void list_destroy(List** list) {
  if (NULL != *list) {
    list_clear(*list);
    free(*list);
    *list = NULL;
  }
}

static int list_length(List* list) {
  assert(NULL != list);

  return list->length;
}

static void list_push_front(List* list, ElementType value) {
  assert(NULL != list);

  {
    Node* node = node_create(value);
    node->next = list->head;
    list->head = node;

    ++list->length;
  }
}

static void list_pop_front(List* list) {
  assert(NULL != list); 

  {
    Node* node = list->head;
    if (NULL != node) {
      list->head = node->next;

      free(node);
      --list->length;
    }
  }
}

static void list_insert(List* list, int index, ElementType value) {
  assert(NULL != list);
  assert(0 <= index && index <= list->length);

  if (0 == index) {
    list_push_front(list, value);
  }
  else {
    int i;
    Node* iter;
    Node* node = node_create(value);

    iter = list->head;
    for (i = 0, iter = list->head; 
        i < index && NULL != iter; ++i, iter = iter->next) {
      if (NULL == iter->next)
        break;
    }

    if (NULL != iter->next)
      node->next = iter->next;
    iter->next = node;

    ++list->length;
  }
}

static void list_remove(List* list, int index) {
  assert(NULL != list);
  assert(0 <= index && index < list->length);

  if (0 == index) {
    list_pop_front(list);
  }
  else {
    int i;
    Node* iter;
    Node* node;

    for (i = 0, iter = list->head; 
        i < index - 1 && NULL != iter->next; ++i, iter = iter->next) {
      if (NULL == iter->next->next)
        break;
    }

    node = iter->next;
    iter->next = node->next;

    free(node);
    --list->length;
  }
}

static int list_find(List* list, ElementType value) {
  assert(NULL != list);

  {
    int i;
    Node* iter;
    for (i = 0, iter = list->head; NULL != iter; ++i, iter = iter->next) {
      if (value == iter->value)
        return i;
    }
  }

  return -1;
}

static void list_show(List* list) {
  assert(NULL != list);

  {
    Node* node = list->head;
    while (NULL != node) {
      fprintf(stdout, 
          "SingleList Node element value = %d\n", node->value);
      node = node->next;
    }
    fprintf(stdout, "\n");
  }
}



void single_list(void) {
  List* list;
  int i;

  fprintf(stdout, "\n\n==========================================\n");

  list = list_init();
  fprintf(stdout, "SingleList length = %d\n", list_length(list));

  for (i = 0; i < 10; ++i)
    list_push_front(list, i);
  fprintf(stdout, "SingleList length = %d\n", list_length(list));
  list_show(list);

  list_insert(list, 2, 222);
  list_insert(list, 0, -111);
  list_insert(list, 12, 1212);
  list_show(list);

  list_pop_front(list);
  list_show(list);

  list_remove(list, 0);
  list_remove(list, 10);
  list_show(list);

  list_remove(list, 3);
  list_show(list);

  fprintf(stdout, "SingleList find (5) = %d\n", list_find(list, 5));

  list_clear(list);
  fprintf(stdout, "SingleList length = %d\n", list_length(list));

  list_destroy(&list);
}
