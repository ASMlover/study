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
#ifndef __SL_ARRAY_HEADER_H__
#define __SL_ARRAY_HEADER_H__

typedef struct sl_array_t sl_array_t;

/*
 * @attention:
 *    All interfaces of array module, 
 *    you must ensure the validity of the 
 *    incoming parameters.
 */


extern sl_array_t* sl_array_create(size_t capacity);
extern void sl_array_release(sl_array_t* array);

extern int sl_array_empty(sl_array_t* array);
extern size_t sl_array_size(sl_array_t* array);
extern size_t sl_array_capacity(sl_array_t* array);
extern void sl_array_clear(sl_array_t* array);
extern void sl_array_pushback(sl_array_t* array, void* value);
extern void* sl_array_popback(sl_array_t* array);
extern void* sl_array_front(sl_array_t* array);
extern void* sl_array_back(sl_array_t* array);
extern void sl_array_set(sl_array_t* array, size_t i, void* value);
extern void* sl_array_get(sl_array_t* array, size_t i);
extern void sl_array_traverse(sl_array_t* array, void (*visit)(void*));

#endif  /* __SL_ARRAY_HEADER_H__ */
