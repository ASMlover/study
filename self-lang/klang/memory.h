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
#ifndef __MEMORY_HEADER_H__
#define __MEMORY_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum KL_MemFailMode {
  /* memory failed mode */
  MEM_FAIL_AND_EXIT,    /* memory failed, then exit */
  MEM_FAIL_AND_RETURN,  /* memory failed, then return */
};

typedef struct KL_MemController KL_MemController;
typedef struct KL_MemStorage KL_MemStorage;
typedef void (*KL_MemErrorHandler)(KL_MemController*, 
    const char*, int, const char*);


extern KL_MemController* g_def_mem_controller;


#ifdef KL_MEM_CONTROLLER
# define KL_MEM_CURRENT_CONTROLLER  KL_MEM_CONTROLLER
#else 
# define KL_MEM_CURRENT_CONTROLLER  g_def_mem_controller
#endif

typedef union KL_MemHeader KL_MemHeader;
struct KL_MemController {
  /* definition of memory controller */
  FILE*               err_file;
  KL_MemErrorHandler  err_handler;
  int                 fail_mode;
  KL_MemHeader*       block_header;
};



/*
 * Don't use KL_mem_*_func functions.
 * They are private functions of memory module.
 */
extern KL_MemController* KL_mem_create_controller(void);

extern void* KL_mem_malloc_func(KL_MemController* controller, 
    const char* filename, int line, size_t size);
extern void* KL_mem_realloc_func(KL_MemController* controller, 
    const char* filename, int line, void* ptr, size_t size);
extern char* KL_mem_strdup_func(KL_MemController* controller, 
    const char* filename, int line, const char* str);
extern void KL_mem_free_func(KL_MemController* controller, void* ptr);


extern KL_MemStorage* KL_mem_open_storage_func(KL_MemController* controller,
    const char* filename, int line, int page_size);
extern void* KL_mem_storage_malloc_func(KL_MemController* controller, 
    const char* filename, int line, KL_MemStorage* storage, size_t size);
extern void KL_mem_dispose_storage_func(KL_MemController* controller,
    KL_MemStorage* storage);


extern void KL_mem_set_error_handler(KL_MemController* controller, 
    KL_MemErrorHandler err_handler);
extern void KL_mem_set_fail_mode(KL_MemController* controller, 
    int fail_mode);


extern void KL_mem_dump_blocks_func(KL_MemController* controller, 
    FILE* fp);
extern void KL_mem_check_block_func(KL_MemController* controller, 
    const char* filename, int line, void* ptr);
extern void KL_mem_check_all_blocks_func(KL_MemController* controller, 
    const char* filename, int line);




/* User's interface */
#define KL_malloc(s)\
  (KL_mem_malloc_func(KL_MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, (s)))
#define KL_realloc(p, s)\
  (KL_mem_realloc_func(KL_MEM_CURRENT_CONTROLLER, \
                       __FILE__, __LINE__, (p), (s)))
#define KL_strdup(str)\
  (KL_mem_strdup_func(KL_MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, (str)))
#define KL_open_storage(page_size)\
  (KL_mem_open_storage_func(KL_MEM_CURRENT_CONTROLLER, \
                            __FILE__, __LINE__, (page_size)))
#define KL_storage_malloc(storage, size)\
  (KL_mem_storage_malloc_func(KL_MEM_CURRENT_CONTROLLER, \
                              __FILE__, __LINE__, (storage), (size)))
#define KL_free(ptr)\
  (KL_mem_free_func(KL_MEM_CURRENT_CONTROLLER, (ptr)))
#define KL_dispose_storage(storage)\
  (KL_mem_dispose_storage_func(KL_MEM_CURRENT_CONTROLLER, (storage)))



#ifdef DEBUG
# define KL_dump_blocks(fp)\
    (KL_mem_dump_blocks_func(KL_MEM_CURRENT_CONTROLLER, (fp)))
# define KL_check_block(p)\
    (KL_mem_check_block_func(KL_MEM_CURRENT_CONTROLLER, \
                             __FILE__, __LINE__, (p)))
#define KL_check_all_blocks()\
    (KL_mem_check_all_blocks_func(KL_MEM_CURRENT_CONTROLLER, \
                                  __FILE__, __LINE__))
#else
# define KL_dump_blocks(fp)     ((void*)0)
# define KL_check_block(p)      ((void*)0)
# define KL_check_all_blocks()  ((void*)0)
#endif


#endif  /* __MEMORY_HEADER_H__ */
