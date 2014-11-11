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
  ElementType value;
  struct ListNode* next;
} ListNode;

typedef struct LoopList {
  ListNode head;
  int length;
} LoopList;


static ListNode* node_create(ElementType value) {
  ListNode* node = (ListNode*)malloc(sizeof(*node));

  if (NULL != node) {
    node->value = value;
    node->next = NULL;
  }

  return node;
}

static void list_clear(LoopList* list) {
  assert(NULL != list);

  {
    ListNode* node;
    while (&list->head != list->head.next) {
      node = list->head.next;
      list->head.next = list->head.next->next;

      free(node);
    }

    list->length = 0;
  }
}

static LoopList* list_init(void) {
  LoopList* list = (LoopList*)malloc(sizeof(*list));

  if (NULL != list) {
    list->head.next = &list->head;
    list->length = 0;
  }

  return list;
}

static void list_destroy(LoopList** list) {
  assert(NULL != *list);

  list_clear(*list);
  free(*list);
  *list = NULL;
}

static int list_length(LoopList* list) {
  assert(NULL != list);

  return list->length;
}

static void list_push(LoopList* list, ElementType value) {
  assert(NULL != list);

  {
    ListNode* node = node_create(value);
    node->next = list->head.next;
    list->head.next = node;

    ++list->length;
  }
}

static void list_pop(LoopList* list) {
  assert(NULL != list);

  {
    ListNode* node = list->head.next;
    if (node != &list->head) {
      list->head.next = list->head.next->next;
      free(node);

      --list->length;
    }
  }
}

static void list_display(LoopList* list, int times) {
  assert(NULL != list);

  {
    ListNode* node = list->head.next;
    int count = 0;
    while (count < times) {
      if (node == &list->head)
        ++count;
      else
        fprintf(stdout, "LoopList element value is : %d\n", node->value);

      node = node->next;
    }
    fprintf(stdout, "\n");
  }
}


void loop_list(void) {
  LoopList* list = list_init();
  int i;
  
  fprintf(stdout, "\n\n==========================================\n");

  fprintf(stdout, "LoopList length = %d\n", list_length(list));

  for (i = 0; i < 10; ++i)
    list_push(list, i * i);
  fprintf(stdout, "LoopList length = %d\n", list_length(list));
  list_display(list, 1);

  list_pop(list);
  list_pop(list);
  list_display(list, 2);
  fprintf(stdout, "LoopList length = %d\n", list_length(list));

  list_clear(list);
  fprintf(stdout, "LoopList length = %d\n", list_length(list));

  list_destroy(&list);
}
