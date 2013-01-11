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
#include <string.h>
#include "array.h"


struct array_s {
  element_t* elements;
  element_t* start;
  element_t* finish;
  element_t* capacity;
};

#define ASSERT_ARRAY(a) {\
  if (NULL == (a)) {\
    fprintf(stderr, "array invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_ARRAY_UNDERFLOW(a) {\
  if ((a)->start == (a)->finish) {\
    fprintf(stderr, "array underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_ARRAY_CHECK_UNDER(i) {\
  if ((i) < 0) {\
    fprintf(stderr, "array checked underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_ARRAY_CHECK(a, i) {\
  if ((i) < 0 || (i) >= ((a)->finish - (a)->start)) {\
    fprintf(stderr, "out of index ...");\
    exit(0);\
  }\
}



array_t 
array_create(int capacity) 
{
  array_t a = (array_t)malloc(sizeof(*a));
  ASSERT_ARRAY(a);

  if (capacity > 0) {
    a->elements = (element_t*)malloc(capacity * sizeof(element_t));
    if (NULL == a->elements) {
      fprintf(stderr, "create array failed ...");
      exit(0);
    }

    a->start  = a->elements;
    a->finish = a->elements + capacity;
    a->capacity = a->finish;
  }
  else {
    a->start = a->finish = NULL;
    a->capacity = NULL;
  }

  return a;
}

void 
array_delete(array_t* a)
{
  if (NULL != *a) {
    array_clear(*a);
    free(*a);
    *a = NULL;
  }
}

int 
array_empty(array_t a) 
{
  ASSERT_ARRAY(a);

  return (a->start == a->finish);
}

int 
array_size(array_t a) 
{
  ASSERT_ARRAY(a);

  return (a->finish - a->start);
}

int 
array_capacity(array_t a) 
{
  ASSERT_ARRAY(a);

  return (a->capacity - a->start);
}

void 
array_clear(array_t a) 
{
  ASSERT_ARRAY(a);

  if (a->start != a->finish) {
    free(a->elements);
    a->start  = a->elements;
    a->finish = a->elements;
    a->capacity = a->elements;
  }
}

void 
array_push_front(array_t a, element_t e)
{
  array_insert(a, 0, e);
}

void 
array_push_back(array_t a, element_t e)
{
  array_insert(a, a->finish - a->start, e);
}

void 
array_insert(array_t a, int i, element_t e)
{
  ASSERT_ARRAY(a);
  ASSERT_ARRAY_CHECK_UNDER(i);

  if (a->finish < a->capacity && a->finish == a->start + i) {
    *a->finish = e;
    ++a->finish;
  }
  else {
    if (a->finish < a->capacity) {
      memmove(a->start + i + 1, a->start + i, 
        (a->finish - a->start - i) * sizeof(element_t));
      ++a->finish;
      *(a->start + i) = e;
    }
    else {
      int old_sz = a->finish - a->start;
      int new_sz = (0 != old_sz ? 2 * old_sz : 1);
      element_t* new_elements = 
        (element_t*)malloc(new_sz * sizeof(element_t));

      if (NULL == new_elements) {
        fprintf(stderr, "array grow failed ...");
        exit(0);
      }

      memcpy(new_elements, a->elements, i * sizeof(element_t));
      *(new_elements + i) = e;
      memcpy(new_elements + (i + 1), (a->elements + i), 
        (a->finish - a->start - i) * sizeof(element_t));

      free(a->elements);
      a->elements = new_elements;
      a->start    = a->elements;
      a->finish   = a->start + old_sz + 1;
      a->capacity = a->start + new_sz;
    }
  }
}

element_t 
array_pop_front(array_t a)
{
  return array_erase(a, 0);
}

element_t 
array_pop_back(array_t a) 
{
  return array_erase(a, a->finish - a->start - 1);
}

element_t 
array_erase(array_t a, int i)
{
  element_t e;

  ASSERT_ARRAY(a);
  ASSERT_ARRAY_CHECK(a, i);

  e = ((a->start + i) == a->finish) ? *(a->finish - 1) : *(a->start + i);
  if ((a->start + i + 1) < a->finish)
    memmove(a->start + i, a->start + i + 1, 
      (a->finish - a->start - i - 1) * sizeof(element_t));
  --a->finish;

  return e;
}

void 
array_set(array_t a, int i, element_t e)
{
  ASSERT_ARRAY(a);
  ASSERT_ARRAY_CHECK(a, i);

  *(a->start + i) = e;
}

element_t 
array_get(array_t a, int i)
{
  ASSERT_ARRAY(a);
  ASSERT_ARRAY_CHECK(a, i);

  return *(a->start + i);
}
