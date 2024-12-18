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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "njmem.h"
#include "njlog.h"
#include "njobject.h"
#include "njdict.h"
#include "njset.h"

#define ALLOC_COUNT (1000000)

static void
njord_memory_pool(void) {
  char* p;
  int alloc_bytes_list[] = {1, 4, 8, 16, 32, 64};
  int alloc_bytes_count = sizeof(alloc_bytes_list) / sizeof(int);

  srand((unsigned int)time(NULL));

  clock_t beg = clock();
  clock_t end = beg;
  njlog_info("[system allocator] begin clock: %ld\n", beg);
  for (int i = 0; i < ALLOC_COUNT; ++i) {
    int s = alloc_bytes_list[rand() % alloc_bytes_count];
    p = (char*)malloc(s);
    free(p);
  }
  end = clock();
  njlog_info("[system allocator] end clock: %ld,  use clock: %ld\n",
      end, end - beg);

  {
    beg = clock();
    njlog_info("[mempool allocator] begin clock: %ld\n", beg);
    for (int i = 0; i < ALLOC_COUNT; ++i) {
      int s = alloc_bytes_list[rand() % alloc_bytes_count];
      p = (char*)njmem_malloc(s);
      njmem_free(p, s);
    }
    end = clock();
    njlog_info("[mempool allocator] end clock: %ld,  use clock: %ld\n",
        end, end - beg);
  }

  njmem_collect();
}

static void
njord_set(void) {
  NjSet* set = njset_create();
  njlog_repr("create set(%p) success, set.size=%d\n",
      set, njset_size(set));

  njset_add(set, (NjObject*)0x77450f80);
  njlog_repr("after set.add, set.size=%d\n", njset_size(set));
  njlog_repr("set.pop=%p\n", njset_pop(set));
  njlog_repr("after set.pop, set.size=%d\n", njset_size(set));

  njset_dealloc(set);
}

static void
njord_dict(void) {
  NjDict* dict = njdict_create();
  njlog_repr("create dict(%p) success, dict.size=%d\n",
      dict, njdict_size(dict));

  int key = 34, value = 134;
  njdict_add(dict, (NjObject*)&key, (NjObject*)&value);
  njlog_repr("after dict.add, dict.size=%d\n", njdict_size(dict));
  NjObject* vobj = njdict_get(dict, (NjObject*)&key);
  njlog_repr("dict.get, item(%p), item.value=%d\n", vobj, *(int*)vobj);
  njdict_pop(dict);
  njlog_repr("after dict.pop, dict.size=%d\n", njdict_size(dict));

  njdict_dealloc(dict);
}

static void
njord_gc_sample1(void) {
  njlog_info("sample1: all objects on stack\n");

  NjObject* vm = njord_new();

  NjObject* i1 = njord_pushint(vm, 1);
  njord_print(i1);
  NjObject* i2 = njord_pushint(vm, 2);
  njord_print(i2);

  njord_pop(vm);
  njord_pop(vm);

  njord_free(vm);
  njmem_collect();
}

static void
njord_gc_sample2(void) {
  njlog_info("sample2: objects nested\n");

  NjObject* vm = njord_new();
  NjObject* i1 = njord_pushint(vm, 1);
  njord_print(i1);
  NjObject* i2 = njord_pushint(vm, 2);
  njord_print(i2);
  NjObject* p1 = njord_pushpair(vm);
  njord_print(p1);
  NjObject* i3 = njord_pushint(vm, 3);
  njord_print(i3);
  NjObject* i4 = njord_pushint(vm, 4);
  njord_print(i4);
  NjObject* p2 = njord_pushpair(vm);
  njord_print(p2);
  NjObject* p3 = njord_pushpair(vm);
  njord_print(p3);

  njord_pop(vm);

  njord_free(vm);
  njmem_collect();
}

static void
njord_gc_sample3(void) {
  njlog_info("sample3: cycle reference objects\n");

  NjObject* vm = njord_new();
  NjObject* i1 = njord_pushint(vm, 1);
  njord_print(i1);
  NjObject* i2 = njord_pushint(vm, 2);
  njord_print(i2);
  NjObject* a = njord_pushpair(vm);
  njord_print(a);
  NjObject* i3 = njord_pushint(vm, 3);
  njord_print(i3);
  NjObject* i4 = njord_pushint(vm, 4);
  njord_print(i4);
  NjObject* b = njord_pushpair(vm);
  njord_print(b);

  njord_setpair(a, b, NULL);
  njord_print(a);
  njord_setpair(b, a, NULL);
  njord_print(b);

  njord_pop(vm);
  njord_pop(vm);

  njord_free(vm);
  njmem_collect();
}

static void
njord_gc_sample4(void) {
  njlog_info("sample4: performance objects\n");

  const int POPCOUNT = 20;
  const int PUSHCOUNT = POPCOUNT * 3;
  NjObject* vm = njord_new();
  for (int i = 0; i < 1000; ++i) {
    for (int j = 0; j < PUSHCOUNT; ++j) {
      if ((j + 1) % 3 == 0)
        njord_debug(njord_pushpair(vm));
      else
        njord_debug(njord_pushint(vm, i));
    }

    for (int j = 0; j < POPCOUNT; ++j)
      njord_pop(vm);
  }
  njord_free(vm);
  njmem_collect();
}

static void
njord_gc(const char* gc_name, int prof) {
  njord_initgc(gc_name);

  njord_gc_sample1();
  njord_gc_sample2();
  njord_gc_sample3();
  if (prof)
    njord_gc_sample4();
}

static void
_njord_usage(void) {
  njlog_repr(
      "USAGE: njord [mem] [set] [dict] [gc] ...\n"
      "USAGE: njord mem - performance memory pool\n"
      "USAGE: njord set - unittest for njord set\n"
      "USAGE: njord dict - unittest for njord dict\n"
      );
  njord_usagegc();
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  if (argc < 2) {
    _njord_usage();
    return 1;
  }

  if (strcmp(argv[1], "mem") == 0) {
    njord_memory_pool();
  }
  else if (strcmp(argv[1], "set") == 0) {
    njord_set();
  }
  else if (strcmp(argv[1], "dict") == 0) {
    njord_dict();
  }
  else if (strcmp(argv[1], "gc") == 0) {
    if (argc < 3) {
      _njord_usage();
      return 1;
    }
    int prof = 0;
    if (argc > 3)
      prof = atoi(argv[3]);
    njord_gc(argv[2], prof);
  }
  else {
    _njord_usage();
  }

  return 0;
}
