/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <string.h>
#include "njlog.h"
#include "njmem.h"
#include "njdict.h"

#define Nj_DICTLEN  (1361)
#define Nj_HASH(ob) ((Nj_size_t)(ob) % Nj_DICTLEN)

struct _dict {
  Nj_ssize_t size;
  NjObject* table[Nj_DICTLEN];
};

NjDict*
njdict_create(void) {
  NjDict* dict = (NjDict*)njmem_malloc(sizeof(NjDict));
  Nj_CHECK(dict != NULL, "create NjDict failed");

  dict->size = 0;
  memset(dict->table, 0, sizeof(dict->table));

  return dict;
}

void
njdict_dealloc(NjDict* dict) {
  njmem_free(dict, sizeof(NjDict));
}

void
njdict_clear(NjDict* dict) {
  dict->size = 0;
  memset(dict->table, 0, sizeof(dict->table));
}

Nj_ssize_t
njdict_size(NjDict* dict) {
  return dict->size;
}

void
njdict_add(NjDict* dict, NjObject* obj) {
  if (obj != NULL) {
    dict->table[Nj_HASH(obj)] = obj;
    ++dict->size;
  }
}

void
njdict_remove(NjDict* dict, NjObject* obj) {
  if (obj != NULL) {
    int i = Nj_HASH(obj);
    if (dict->table[i] != NULL) {
      dict->table[i] = NULL;
      --dict->size;
    }
  }
}

NjObject*
njdict_pop(NjDict* dict) {
  NjObject* obj = NULL;
  for (int i = 0; i < Nj_DICTLEN; ++i) {
    if (dict->table[i] != NULL) {
      obj = dict->table[i];
      dict->table[i] = NULL;
      --dict->size;
      break;
    }
  }
  return obj;
}

void
njdict_traverse(NjDict* dict, visitfunc visit, void* arg) {
  for (int i = 0; i < Nj_DICTLEN; ++i) {
    if (dict->table[i] != NULL)
      visit(dict->table[i], arg);
  }
}
