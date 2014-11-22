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
#include <string.h>

typedef int ElementType;

typedef struct SequentialList {
  int length;
  int storage;
  ElementType* elements;
} SequentialList;

#define DEF_STORAGE (128)

static SequentialList* sequential_init(int storage) {
  SequentialList* list = (SequentialList*)malloc(sizeof(*list));
  if (NULL == list)
    return NULL;

  if (storage < DEF_STORAGE)
    storage = DEF_STORAGE;

  list->elements = (ElementType*)malloc(sizeof(ElementType) * storage);
  assert(NULL != list->elements);
  list->length = 0;
  list->storage = storage;

  return list;
}

static void sequential_destroy(SequentialList** list) {
  assert(NULL != *list && NULL != (*list)->elements);

  free((*list)->elements);
  free(*list);
  *list = NULL;
}

static void sequential_clear(SequentialList* list) {
  assert(NULL != list && NULL != list->elements);
  list->length = 0;
}

static int sequential_length(SequentialList* list) {
  assert(NULL != list && NULL != list->elements);

  return list->length;
}

static ElementType sequential_get(
    SequentialList* list, int index) {
  assert(NULL != list && NULL != list->elements);
  assert(0 <= index && index < list->length);

  return list->elements[index];
}

static void sequential_set(
    SequentialList* list, int index, ElementType value) {
  assert(NULL != list && NULL != list->elements);
  assert(0 <= index && index < list->length);

  list->elements[index] = value;
}

static void sequential_insert(
    SequentialList* list, int index, ElementType value) {
  assert(NULL != list && NULL != list->elements);
  assert(0 <= index 
      && index <= list->length 
      && list->length < list->storage);

  if (index == list->length) {
    list->elements[list->length] = value;
  }
  else {
    int i;
    for (i = list->length - 1; i >= index; --i)
      list->elements[i + 1] = list->elements[i];
    list->elements[index] = value;
  }
  ++list->length;
}

static void sequential_remove(
    SequentialList* list, int index) {
  assert(NULL != list && NULL != list->elements);
  assert(0 <= index && index < list->length);

  {
    int i;
    for (i = index; i < list->length - 1; ++i)
      list->elements[i] = list->elements[i + 1];
    --list->length;
  }
}

static int sequential_find(
    SequentialList* list, ElementType value) {
  assert(NULL != list && NULL != list->elements);

  {
    int i;
    for (i = 0; i < list->length; ++i) {
      if (value == list->elements[i])
        return i;
    }
  }

  return -1;
}



void sequential_list(void) {
  SequentialList* list = sequential_init(10);
  int i, len;
  ElementType v;
  
  fprintf(stdout, "\n\n==========================================\n");

  fprintf(stdout, "SequentialList length = %d\n", sequential_length(list));
  for (i = 0; i < 10; ++i)
    sequential_insert(list, i, i * i);

  len = sequential_length(list);
  fprintf(stdout, "SequentialList length = %d\n", len);
  for (i = 0; i < len; ++i) {
    v = sequential_get(list, i);
    fprintf(stdout, "SequentialList [%d] = %d\n", i, v);
  }
  fprintf(stdout, "\n");
  
  sequential_set(list, 0, 333);
  for (i = 0; i < len; ++i) {
    v = sequential_get(list, i);
    fprintf(stdout, "SequentialList [%d] = %d\n", i, v);
  }
  fprintf(stdout, "\n");

  sequential_remove(list, 0);
  len = sequential_length(list);
  for (i = 0; i < len; ++i) {
    v = sequential_get(list, i);
    fprintf(stdout, "SequentialList [%d] = %d\n", i, v);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "SequentialList find (9) = %d\n", 
      sequential_find(list, 9));

  sequential_clear(list);
  fprintf(stdout, "SequentialList length = %d\n", sequential_length(list));

  sequential_destroy(&list);
}
