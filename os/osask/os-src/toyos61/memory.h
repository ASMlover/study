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
#ifndef __MEMORY_HEADER_H__
#define __MEMORY_HEADER_H__ 

#define MEMMGR_FREES        (4090)  /* about 32KB */
#define MEMMGR_ADDR         (0x003c0000)


typedef struct free_info_t {  /* available information */
  unsigned int addr;
  unsigned int size;
} free_info_t;

typedef struct mem_mgr_t {
  int frees;      /* available information number */
  int max_frees;  /* max value of free */
  int fail_frees; /* total memory size of free failed */
  int fail_num;   /* number of free failed */
  free_info_t free_list[MEMMGR_FREES];
} mem_mgr_t;

extern unsigned int memory_test(unsigned int start, unsigned int end);
extern void mem_mgr_init(mem_mgr_t* mgr);
extern unsigned int mem_mgr_total(mem_mgr_t* mgr);
extern unsigned int mem_mgr_alloc(mem_mgr_t* mgr, unsigned int size);
extern int mem_mgr_free(mem_mgr_t* mgr, 
    unsigned int addr, unsigned int size);
extern unsigned int mem_mgr_alloc_4k(mem_mgr_t* mgr, unsigned int size);
extern int mem_mgr_free_4k(mem_mgr_t* mgr, 
    unsigned int addr, unsigned int size);

#endif  /* __MEMORY_HEADER_H__ */
