// Copyright (c) 2024 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <cstdlib>
#include <cstring>
#include <new>

#include "owl_type_traits.hh"
#include "owl_iterator.hh"

#if !defined(THROW_BAD_ALLOC)
# define THROW_BAD_ALLOC  throw std::bad_alloc()
#endif

#if !defined(_BAD_ALLOC)
# define _BAD_ALLOC       std::bad_alloc
#endif

namespace owl {

template <int inst> class MallocAllocTemplate {
  static void* oom_malloc(size_t n);
  static void* oom_realloc(void* p, size_t n);
  static void (*malloc_alloc_oom_handler)();
public:
  static void* allocate(size_t n) {
    void* result = malloc(n);
    if (nullptr == result)
      result = oom_malloc(n);
    return result;
  }

  static void deallocate(void* p, size_t) {
    free(p);
  }

  static void* reallocate(void* p, size_t, size_t new_size) {
    void* result = realloc(p, new_size);
    if (nullptr == result)
      result = oom_realloc(p, new_size);
    return result;
  }

  static void (*set_malloc_handler(void (*f)()))() {
    void (*old)() = malloc_alloc_oom_handler;
    malloc_alloc_oom_handler = f;
    return old;
  }
};

template <int inst>
void (*MallocAllocTemplate<inst>::malloc_alloc_oom_handler)() = nullptr;

template <int inst> void* MallocAllocTemplate<inst>::oom_malloc(size_t n) {
  void (*my_malloc_handler)();
  void* result;

  for (;;) {
    my_malloc_handler = malloc_alloc_oom_handler;
    if (nullptr == my_malloc_handler)
      THROW_BAD_ALLOC;

    (*my_malloc_handler)();
    result = malloc(n);
    if (nullptr != result)
      return result;
  }
  return nullptr;
}

template <int inst> void* MallocAllocTemplate<inst>::oom_realloc(void* p, size_t n) {
  void (*my_malloc_handler)();
  void* result;
  for (;;) {
    my_malloc_handler = malloc_alloc_oom_handler;
    if (nullptr == my_malloc_handler)
      THROW_BAD_ALLOC;
    (*my_malloc_handler)();
    result = realloc(p, n);
    if (nullptr != result)
      return result;
  }
  return nullptr;
}

using MallocAlloc = MallocAllocTemplate<0>;

enum {
  ALIGN         = 8,
  MAX_BYTES     = 128,
  NFREELISTS    = MAX_BYTES / ALIGN,
};

template <bool threads, int inst> class DefaultAllocTemplate {
  static char* start_free_;
  static char* end_free_;
  static size_t heap_size_;

  static size_t round_up(size_t bytes) noexcept {
    return (((bytes) + ALIGN - 1) & (~(ALIGN - 1)));
  }

  union Obj {
    union Obj* free_list_link;
    char client_data[1];
  };

  static Obj* volatile free_list_[NFREELISTS];

  static size_t free_list_index(size_t bytes) noexcept {
    return (((bytes) + ALIGN - 1) / ALIGN - 1);
  }

  static void* refill(size_t n);
  static void* chunk_alloc(size_t size, size_t& nobjs);
public:
  static void* allocate(size_t n) {
    if (n > (size_t)MAX_BYTES)
      return MallocAlloc::allocate(n);

    Obj* volatile* my_free_list = free_list_ + free_list_index(n);
    Obj* result = *my_free_list;
    if (nullptr == result) {
      void* r = refill(round_up(n));
      return r;
    }
    *my_free_list = result->free_list_link;
    return result;
  }

  static void deallocate(void* p, size_t n) {
    if (n > (size_t)MAX_BYTES) {
      MallocAlloc::deallocate(p, n);
      return;
    }

    Obj* q = (Obj*)p;
    Obj* volatile* my_free_list = free_list_ + free_list_index(n);
    q->free_list_link = *my_free_list;
    *my_free_list = q;
  }
};

template <bool threads, int inst> char* DefaultAllocTemplate<threads, inst>::start_free_ = nullptr;
template <bool threads, int inst> char* DefaultAllocTemplate<threads, inst>::end_free_ = nullptr;
template <bool threads, int inst> size_t DefaultAllocTemplate<threads, inst>::heap_size_ = 0;

template <bool threads, int inst>
typename DefaultAllocTemplate<threads, inst>::Obj* volatile
DefaultAllocTemplate<threads, inst>::free_list_[NFREELISTS] = {
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

template <bool threads, int inst>
void* DefaultAllocTemplate<threads, inst>::refill(size_t n) {
  size_t nobjs = 20;
  char* chunk = chunk_alloc(n, nobjs);
  if (1 == nobjs)
    return chunk;

  Obj* volatile* my_free_list = free_list_ + free_list_index(n);
  Obj* result = (Obj*)chunk;
  Obj* cur_obj;
  Obj* next_obj;
  *my_free_list = next_obj = (Obj*)(chunk + n);
  for (size_t i = 1; ; ++i) {
    if (nobjs - 1 == i) {
      cur_obj->free_list_link = nullptr;
      break;
    }
    else {
      next_obj = (Obj*)((char*)next_obj + n);
      cur_obj->free_list_link = next_obj;
    }
  }

  return result;
}

}
