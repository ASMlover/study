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
#include "global.h"
#include "memory.h"



typedef union KL_Cell {
  long    l_dummy;
  double  d_dummy;
  void*   p_dummy;
} KL_Cell;


#define KL_CELL_SIZE      (sizeof(KL_Cell))
#define KL_DEF_PAGE_SIZE  (1024)            /* number of cell */


typedef struct KL_MemPage {
  /* definition of memory page */
  struct KL_MemPage*  next;
  int                 cell_num;
  int                 use_cell_num;
  KL_Cell             cell[1];
} KL_MemPage;

struct KL_MemStorage {
  /* definition of memory storage */
  KL_MemPage* page_list;
  int         current_page_size;
};





KL_MemStorage* 
KL_mem_open_storage_func(KL_MemController* controller, 
    const char* filename, int line, int page_size)
{
  KL_MemStorage* storage = KL_mem_malloc_func(
      controller, filename, line, sizeof(*storage));
  storage->page_list = NULL;

  assert(page_size >= 0);
  if (page_size > 0)
    storage->current_page_size = page_size;
  else 
    storage->current_page_size = KL_DEF_PAGE_SIZE;

  return storage;
}

void* 
KL_mem_storage_malloc_func(KL_MemController* controller, 
    const char* filename, int line, KL_MemStorage* storage, size_t size)
{
  int   cell_num;
  void* ptr;

  cell_num = ((size - 1) / KL_CELL_SIZE) + 1;

  if (NULL != storage->page_list 
      && (storage->page_list->use_cell_num + cell_num 
        < storage->page_list->cell_num)) {
    ptr = &(storage->page_list->cell[storage->page_list->use_cell_num]);
    storage->page_list->use_cell_num += cell_num;
  }
  else {
    int alloc_cell_num = larger(cell_num, storage->current_page_size);
    KL_MemPage* new_page = KL_mem_malloc_func(
        controller, filename, line, 
        sizeof(*new_page) + KL_CELL_SIZE * (alloc_cell_num -1));

    new_page->next = storage->page_list;
    new_page->cell_num = alloc_cell_num;
    storage->page_list = new_page;

    ptr = &(new_page->cell[0]);
    new_page->use_cell_num = cell_num;
  }

  return ptr;
}

void 
KL_mem_dispose_storage_func(KL_MemController* controller, 
    KL_MemStorage* storage)
{
  KL_MemPage* mem_page;

  while (NULL != storage->page_list) {
    mem_page = storage->page_list;
    storage->page_list = storage->page_list->next;
    KL_mem_free_func(controller, mem_page);
  }

  KL_mem_free_func(controller, storage);
}
