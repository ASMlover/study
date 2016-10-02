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
#ifndef DEVIL_LIST_HEADER_H
#define DEVIL_LIST_HEADER_H

typedef struct devil_list_t devil_list_t;
typedef struct devil_list_node_t* devil_list_iter_t;

/*
 * @attention:
 *    All interfaces of list module,
 *    you must ensure the validity of the
 *    incoming parameters.
 */

devil_list_t* devil_list_create(void);
void devil_list_release(devil_list_t* list);

int devil_list_empty(devil_list_t* list);
size_t devil_list_size(devil_list_t* list);
void devil_list_clear(devil_list_t* list);
void devil_list_pushback(devil_list_t* list, void* value);
void devil_list_pushfront(devil_list_t* list, void* value);
void* devil_list_popback(devil_list_t* list);
void* devil_list_popfront(devil_list_t* list);
void* devil_list_front(devil_list_t* list);
void* devil_list_back(devil_list_t* list);
void devil_list_traverse(devil_list_t* list, void (*visitor)(void*));
devil_list_iter_t devil_list_begin(devil_list_t* list);
devil_list_iter_t devil_list_end(devil_list_t* list);
devil_list_iter_t devil_list_iter_next(devil_list_iter_t iter);
devil_list_iter_t devil_list_iter_prev(devil_list_iter_t iter);
void* devil_list_iter_value(devil_list_iter_t iter);

#endif  /* DEVIL_LIST_HEADER_H */
