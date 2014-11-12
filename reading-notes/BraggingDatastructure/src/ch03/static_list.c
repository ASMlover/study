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

#define DEF_STORAGE (128)

typedef int ElementType;

typedef struct Node {
  ElementType value;
  int next;
} Node;

typedef struct List {
  int length;
  int storage;
  int first;
  Node* elements;
} List;

static List* list_init(int storage) {
  List* list = (List*)malloc(sizeof(*list));
  if (NULL == list)
    return NULL;

  list->length = 0;
  list->storage = (storage < DEF_STORAGE ? DEF_STORAGE : storage);
  list->first = 0;
  list->elements = (Node*)malloc(sizeof(Node) * list->storage);
  if (NULL == list->elements) {
    free(list);
    list = NULL;
  }
  else {
    int i;
    for (i = 0; i < list->storage - 1; ++i)
      list->elements[i].next = i + 1;
    list->elements[i].next = 0;
  }

  return list;
}

static void list_destroy(List** list) {
  assert(NULL != *list);

  if (NULL != (*list)->elements) 
    free((*list)->elements);
  free(*list);
  *list = NULL;
}

static void list_clear(List* list) {
  assert(NULL != list);

  list->length = 0;
  list->first = 0;
}

static int list_length(List* list) {
  assert(NULL != list);

  return list->length;
}

static void list_push(List* list, ElementType value) {
  assert(NULL != list && list->length < list->storage - 1);

  {
    int mark = list->storage - 1;
    int i = list->elements[mark].next;
    list->elements[i].value = value;
    list->elements[mark].next = list->elements[i].next;

    ++list->length;
  }
}

static void list_pop(List* list) {
  assert(NULL != list && list->length > 0);

  {
    int mark = list->storage - 1;
    int old = list->first;
    list->first = list->elements[old].next;
    list->elements[old].next = list->elements[mark].next;
    list->elements[mark].next = old;

    --list->length;
  }
}

static void list_display(List* list) {
  assert(NULL != list);

  {
    int index;
    int length = 0;
    for (index = list->first; length < list->length; ++length) {
      fprintf(stdout, "StaticList node value is : %d\n", 
          list->elements[index].value);
      index = list->elements[index].next;
    }
    fprintf(stdout, "\n");
  }
}


void static_list(void) {
  List* list = list_init(128);
  int i;

  fprintf(stdout, "\n\n==========================================\n");

  fprintf(stdout, "StaticList length = %d\n", list_length(list));

  for (i = 0; i < 10; ++i)
    list_push(list, i * 2);
  list_push(list, 12121);
  fprintf(stdout, "StaticList length = %d\n", list_length(list));
  list_display(list);

  list_pop(list);
  list_pop(list);
  fprintf(stdout, "StaticList length = %d\n", list_length(list));
  list_display(list);

  list_clear(list);
  fprintf(stdout, "StaticList length = %d\n", list_length(list));

  list_destroy(&list);
}
