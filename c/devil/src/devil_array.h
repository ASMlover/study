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
#ifndef DEVIL_ARRAY_HEADER_H
#define DEVIL_ARRAY_HEADER_H

typedef struct devil_array_t devil_array_t;

/*
 * @attention:
 *    All interfaces of array module,
 *    you must ensure the validity of the
 *    incoming parameters.
 */

devil_array_t* devil_array_create(size_t capacity);
void devil_array_release(devil_array_t* array);

int devil_array_empty(devil_array_t* array);
size_t devil_array_size(devil_array_t* array);
size_t devil_array_capacity(devil_array_t* array);
void devil_array_clear(devil_array_t* array);
void devil_array_pushback(devil_array_t* array, void* value);
void* devil_array_popback(devil_array_t* array);
void* devil_array_front(devil_array_t* array);
void* devil_array_back(devil_array_t* array);
void devil_array_set(devil_array_t* array, size_t i, void* value);
void* devil_array_get(devil_array_t* array, size_t i);
void devil_array_traverse(devil_array_t* array, void (*visitor)(void*));

#endif  /* DEVIL_ARRAY_HEADER_H */
