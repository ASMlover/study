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
#include <limits.h>
#include <string.h>
#include "sl_allocator.h"
#include "sl_table.h"


#if defined(_WINDOWS_) || defined(_MSC_VER)
  #define inline __inline
#endif

#define streq(s1, s2)   (0 == strcmp(s1, s2))



typedef struct sl_table_item_t sl_table_item_t;
struct sl_table_item_t {
  sl_table_item_t* prev;
  sl_table_item_t* next;
  const char* key;
  void* value;
  void (*release)(void*);
};

struct sl_table_t {
  sl_table_item_t* items;
  size_t limit;
  size_t size;
};



static inline unsigned int 
sl_table_item_hash(const char* key, size_t limit)
{
  unsigned int hash_key = 0;
  while ('\0' != *key)
    hash_key = 33 * hash_key ^ *key++;
  hash_key %= limit;

  return hash_key;
}

static void 
sl_table_insert(sl_table_t* table, sl_table_item_t* pos, 
    const char* key, void* value, void (*release)(void*))
{
  sl_table_item_t* item = 
    (sl_table_item_t*)sl_malloc(sizeof(sl_table_item_t));
  item->key = key;
  item->value = value;
  item->release = release;

  item->prev = pos->prev;
  item->next = pos;
  pos->prev->next = item;
  pos->prev = item;

  ++table->size;
}

static void
sl_table_erase(sl_table_t* table, sl_table_item_t* pos)
{
  sl_table_item_t* prev = pos->prev;
  sl_table_item_t* next = pos->next;

  prev->next = next;
  next->prev = prev;

  if (NULL != pos->release)
    pos->release(pos->value);
  sl_free(pos);
  --table->size;
}






sl_table_t* 
sl_table_create(size_t limit)
{
  static size_t primes[] = {
    509, 509, 1021, 2053, 4093, 8191, 18381, 32771, 65521, INT_MAX
  };
  size_t i;
  sl_table_t* table = (sl_table_t*)sl_malloc(sizeof(sl_table_t));
  assert(NULL != table);

  for (i = 1; primes[i] < limit; ++i) {
  }
  table->limit = primes[i - 1];
  table->items = 
    (sl_table_item_t*)sl_malloc(sizeof(sl_table_item_t) * table->limit);
  for (i = 0; i < table->limit; ++i) {
    table->items[i].prev = &table->items[i];
    table->items[i].next = &table->items[i];
    table->items[i].key = NULL;
    table->items[i].value = NULL;
    table->items[i].release = NULL;
  }
  table->size = 0;

  return table;
}

void 
sl_table_release(sl_table_t* table)
{
  size_t i;
  sl_table_item_t* iter;
  sl_table_item_t* item;
  for (i = 0; i < table->limit; ++i) {
    iter = table->items[i].next;
    while (iter != &table->items[i]) {
      item = iter;
      iter = iter->next;

      if (NULL != item->release)
        item->release(item->value);
      sl_free(item);
    }
  }
  sl_free(table->items);
  sl_free(table);
}

size_t 
sl_table_size(sl_table_t* table)
{
  return table->size;
}

int 
sl_table_exsits(sl_table_t* table, const char* key)
{
  unsigned int hash_key = sl_table_item_hash(key, table->limit);
  sl_table_item_t* item = table->items[hash_key].next;

  int found = 0;
  while (item != &table->items[hash_key]) {
    if (streq(key, item->key)) {
      found = 1;
      break;
    }

    item = item->next;
  }

  return found;
}

void 
sl_table_set(sl_table_t* table, 
    const char* key, void* value, void (*release)(void*))
{
  unsigned int hash_key = sl_table_item_hash(key, table->limit);
  sl_table_item_t* item = table->items[hash_key].next;

  int found = 0;
  while (item != &table->items[hash_key]) {
    if (streq(key, item->key)) {
      if (NULL != item->release)
        item->release(item->value);

      item->value = value;
      item->release = release;

      found = 1;
      break;
    }

    item = item->next;
  }

  if (!found)
    sl_table_insert(table, &table->items[hash_key], key, value, release);
}

void* 
sl_table_get(sl_table_t* table, const char* key)
{
  unsigned int hash_key = sl_table_item_hash(key, table->limit);
  sl_table_item_t* item = table->items[hash_key].next;
  void* value = NULL;

  while (item != &table->items[hash_key]) {
    if (streq(key, item->key)) {
      value = item->value;
      break;
    }

    item = item->next;
  }

  return value;
}

void 
sl_table_remove(sl_table_t* table, const char* key)
{
  unsigned int hash_key = sl_table_item_hash(key, table->limit);
  sl_table_item_t* item = table->items[hash_key].next;

  while (item != &table->items[hash_key]) {
    if (streq(key, item->key)) {
      sl_table_erase(table, item);
      break;
    }

    item = item->next;
  }
}

void 
sl_table_traverse(sl_table_t* table, void (*visit)(const char*, void*))
{
  size_t i;
  sl_table_item_t* item;

  if (NULL == visit)
    return;
  for (i = 0; i < table->limit; ++i) {
    item = table->items[i].next;

    while (item != &table->items[i]) {
      visit(item->key, item->value);

      item = item->next;
    }
  }
}
