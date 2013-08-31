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
#ifndef __SL_LIST_HEADER_H__
#define __SL_LIST_HEADER_H__

typedef struct sl_list_t sl_list_t;
typedef struct sl_list_node_t sl_list_iter_t;


/*
 * @attention:
 *    All interfaces of list module, 
 *    you must ensure the validity of the 
 *    incoming parameters.
 */

extern sl_list_t* sl_list_create(void);
extern void sl_list_release(sl_list_t* list);

extern int sl_list_empty(sl_list_t* list);
extern size_t sl_list_size(sl_list_t* list);
extern void sl_list_clear(sl_list_t* list);
extern void sl_list_pushback(sl_list_t* list, void* value);
extern void sl_list_pushfront(sl_list_t* list, void* value);
extern void* sl_list_popback(sl_list_t* list);
extern void* sl_list_popfront(sl_list_t* list);
extern void* sl_list_front(sl_list_t* list);
extern void* sl_list_back(sl_list_t* list);
extern void sl_list_traverse(sl_list_t* list, void (*visit)(void*));
extern sl_list_iter_t* sl_list_begin(sl_list_t* list);
extern sl_list_iter_t* sl_list_end(sl_list_t* list);
extern sl_list_iter_t* sl_list_iter_next(sl_list_iter_t* iter);
extern sl_list_iter_t* sl_list_iter_prev(sl_list_iter_t* iter);
extern void* sl_list_iter_value(sl_list_iter_t* iter);


#endif  /* __SL_LIST_HEADER_H__ */
