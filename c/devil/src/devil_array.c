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
#include <assert.h>
#include <string.h>
#include "devil_config.h"
#include "devil_allocator.h"
#include "devil_array.h"

enum devil_array_size_type {
  DEF_CAPACITY = 128,
};

struct devil_array_t {
  void** elems;
  void** start;
  void** finish;
  size_t capacity;
};

static inline void
devil_array_regrow(devil_array_t* array)
{
  size_t old_size = array->finish - array->start;
  size_t new_capacity = (size_t)(array->capacity * 1.5);
  void** new_elems = (void**)devil_malloc(sizeof(void*) * new_capacity);
  assert(NULL != new_elems);

  memmove(new_elems, array->elems, sizeof(void*) * old_size);
  devil_free(array->elems);
  array->elems = new_elems;
  array->start = array->elems;
  array->finish = array->elems + old_size;
  array->capacity = new_capacity;
}

devil_array_t*
devil_array_create(size_t capacity)
{
  devil_array_t* array = (devil_array_t*)devil_malloc(sizeof(devil_array_t));
  assert(NULL != array);

  if (capacity < DEF_CAPACITY)
    capacity = DEF_CAPACITY;
  array->elems = (void**)devil_malloc(sizeof(void*) * capacity);
  assert(NULL != array->elems);
  array->start = array->elems;
  array->finish = array->elems;
  array->capacity = capacity;

  return array;
}

void
devil_array_release(devil_array_t* array)
{
  devil_free(array->elems);
  devil_free(array);
}

int
devil_array_empty(devil_array_t* array)
{
  return (array->start == array->finish);
}

size_t
devil_array_size(devil_array_t* array)
{
  return (array->finish - array->start);
}

size_t
devil_array_capacity(devil_array_t* array)
{
  return array->capacity;
}

void
devil_array_clear(devil_array_t* array)
{
  /* free space back to array but not free the memory */
  array->finish = array->elems;
}

void
devil_array_pushback(devil_array_t* array, void* value)
{
  if ((size_t)(array->finish - array->start) >= array->capacity)
    devil_array_regrow(array);

  *array->finish = value;
  ++array->finish;
}

void*
devil_array_popback(devil_array_t* array)
{
  void* value;
  if (array->start == array->finish)
    value = NULL;
  else
    value = *(--array->finish);

  return value;
}

void*
devil_array_front(devil_array_t* array)
{
  if (array->finish > array->start)
    return *array->start;

  return NULL;
}

void*
devil_array_back(devil_array_t* array)
{
  if (array->finish > array->start)
    return *(array->finish - 1);

  return NULL;
}

void
devil_array_set(devil_array_t* array, size_t i, void* value)
{
  if (array->start + i >= array->finish)
    return;

  *(array->start + i) = value;
}

void*
devil_array_get(devil_array_t* array, size_t i)
{
  if (array->start + i >= array->finish)
    return NULL;

  return *(array->start + i);
}

void
devil_array_traverse(devil_array_t* array, void (*visitor)(void*))
{
  void** iter;

  if (NULL == visitor)
    return;
  iter = array->start;
  while (iter < array->finish) {
    visitor(*iter);

    ++iter;
  }
}
