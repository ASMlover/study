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
#ifndef DEVIL_TABLE_HEADER_H
#define DEVIL_TABLE_HEADER_H

typedef struct devil_table_t devil_table_t;

/*
 * @attention:
 *    All interfaces of table module,
 *    you must ensure the validity of the
 *    incoming parameters.
 */

devil_table_t* devil_table_create(size_t limit);
void devil_table_release(devil_table_t* table);

void devil_table_clear(devil_table_t* table);
size_t devil_table_size(devil_table_t* table);
int devil_table_exsits(devil_table_t* table, const char* key);
void devil_table_set(devil_table_t* table, const char* key, void* value, void (*release)(void*));
void* devil_table_get(devil_table_t* table, const char* key);
void devil_table_remove(devil_table_t* table, const char* key);

/*
 * @attention:
 *    You should careful to use this interface,
 *    it may cost your more time O(m * n);
 */
void devil_table_traverse(devil_table_t* table, void (*visitor)(const char*, void*));

#endif  /* DEVIL_TABLE_HEADER_H */
