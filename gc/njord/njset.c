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
#include "njset.h"

#define Nj_SETLEN    (1361)
#define Nj_HASH(ob)  ((Nj_size_t)(ob) % Nj_SETLEN)
#define Nj_CLRSET(s) do {\
  (s)->size = 0;\
  memset((s)->table, 0, sizeof((s)->table));\
} while (0)

struct _set {
  Nj_ssize_t size;
  NjObject* table[Nj_SETLEN];
};

NjSet*
njset_create(void) {
  NjSet* set = (NjSet*)njmem_malloc(sizeof(NjSet));
  Nj_CHECK(set != NULL, "create NjSet failed");

  Nj_CLRSET(set);
  return set;
}

void
njset_dealloc(NjSet* set) {
  njmem_free(set, sizeof(NjSet));
}

void
njset_clear(NjSet* set) {
  Nj_CLRSET(set);
}

Nj_ssize_t
njset_size(NjSet* set) {
  return set->size;
}

Nj_bool_t
njset_contains(NjSet* set, NjObject* obj) {
  return set->table[Nj_HASH(obj)] != NULL;
}

void
njset_add(NjSet* set, NjObject* obj) {
  if (obj != NULL) {
    set->table[Nj_HASH(obj)] = obj;
    ++set->size;
  }
}

void
njset_remove(NjSet* set, NjObject* obj) {
  if (obj != NULL) {
    int i = Nj_HASH(obj);
    if (set->table[i] != NULL) {
      set->table[i] = NULL;
      --set->size;
    }
  }
}

NjObject*
njset_pop(NjSet* set) {
  NjObject* obj = NULL;
  for (int i = 0; i < Nj_SETLEN; ++i) {
    if (set->table[i] != NULL) {
      obj = set->table[i];
      set->table[i] = NULL;
      --set->size;
      break;
    }
  }
  return obj;
}

void
njset_traverse(NjSet* set, visitfunc visit, void* arg) {
  for (int i = 0; i < Nj_SETLEN; ++i) {
    if (set->table[i] != NULL)
      visit(set->table[i], arg);
  }
}
