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
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
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
#include "common.h"
#include "memory.h"



#define EFLAGS_AC_BIT       (0x00040000)
#define CR0_CACHE_DISABLE   (0x60000000)

unsigned int 
memory_test(unsigned int start, unsigned int end)
{
  char flag486 = 0;
  unsigned int eflags, cr0, address;

  /* check the type of CPU (386? 486?) */
  eflags = io_load_eflags();
  eflags |= EFLAGS_AC_BIT;  /* AC-bit = 1 */
  io_store_eflags(eflags);
  if (0 != (eflags & EFLAGS_AC_BIT))  /* if 386, set AC=1, AC back to 0 */
    flag486 = 1;
  eflags &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
  io_store_eflags(eflags);

  if (0 != flag486) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;   /* disable cache */
    store_cr0(cr0);
  }

  address = memory_test_sub(start, end);

  if (0 != flag486) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;  /* allow cache */
    store_cr0(cr0);
  }

  return address;
}
  



void 
mem_mgr_init(mem_mgr_t* mgr)
{
  mgr->frees      = 0;
  mgr->max_frees  = 0;
  mgr->fail_frees = 0;
  mgr->fail_num   = 0;
}

unsigned int 
mem_mgr_total(mem_mgr_t* mgr)
{
  /* get free memory size */
  unsigned int i, total = 0;
  for (i = 0; i < mgr->frees; ++i)
    total += mgr->free_list[i].size;

  return total;
}

unsigned int 
mem_mgr_alloc(mem_mgr_t* mgr, unsigned int size)
{
  unsigned int i, address;

  for (i = 0; i < mgr->frees; ++i) {
    if (mgr->free_list[i].size >= size) {
      /* find enough memory space */
      address = mgr->free_list[i].addr;
      mgr->free_list[i].addr += size;
      mgr->free_list[i].size -= size;

      if (0 == mgr->free_list[i].size) {
        /* take away an available memory information */
        --mgr->frees;
        for ( ; i < mgr->frees; ++i)
          mgr->free_list[i] = mgr->free_list[i + 1];
      }

      return address;
    }
  }

  return 0;   /* no space can be used */
}

int 
mem_mgr_free(mem_mgr_t* mgr, unsigned int addr, unsigned int size)
{
  int i, j;

  /* arrangement of memory order by address */
  for (i = 0; i < mgr->frees; ++i) {
    if (mgr->free_list[i].addr > addr)
      break;
  }

  /* if free_list[i - 1].addr < addr < free_list[i].addr */
  if (i > 0) {
    if (mgr->free_list[i - 1].addr + mgr->free_list[i - 1].size == addr) {
      mgr->free_list[i - 1].size += size;
      if (i < mgr->frees) {
        if (addr + size == mgr->free_list[i].addr) {
          mgr->free_list[i - 1].size += mgr->free_list[i].size;
          --mgr->frees;
          for ( ; i < mgr->frees; ++i)
            mgr->free_list[i] = mgr->free_list[i + 1];
        }
      }

      return 0; /* success */
    }
  }

  /* can't be merged together with front */
  if (i < mgr->frees) {
    if (addr + size == mgr->free_list[i].addr) {
      mgr->free_list[i].addr = addr;
      mgr->free_list[i].size += size;

      return 0;
    }
  }

  /* can't be merged together with front and back */
  if (mgr->frees < MEMMGR_FREES) {
    for (j = mgr->frees; j > i; --j) 
      mgr->free_list[j] = mgr->free_list[j - 1];
    
    ++mgr->frees;
    if (mgr->max_frees < mgr->frees)
      mgr->max_frees = mgr->frees;

    mgr->free_list[i].addr = addr;
    mgr->free_list[i].size = size;
    return 0;
  }

  /* can't move to back */
  ++mgr->fail_num;
  mgr->fail_frees += size;
  return -1;
}

unsigned int 
mem_mgr_alloc_4k(mem_mgr_t* mgr, unsigned int size)
{
  unsigned int address;

  size = (size + 0xfff) & 0xfffff000;
  address = mem_mgr_alloc(mgr, size);

  return address;
}

int 
mem_mgr_free_4k(mem_mgr_t* mgr, unsigned int addr, unsigned int size)
{
  int ret;

  size = (size + 0xfff) & 0xfffff000;
  ret = mem_mgr_free(mgr, addr, size);

  return ret;
}
