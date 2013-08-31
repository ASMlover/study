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
#ifndef __SL_TABLE_HEADER_H__
#define __SL_TABLE_HEADER_H__

typedef struct sl_table_t sl_table_t;

/*
 * @attention:
 *    All interfaces of table module, 
 *    you must ensure the validity of the 
 *    incoming parameters.
 */


extern sl_table_t* sl_table_create(size_t limit);
extern void sl_table_release(sl_table_t* table);

extern size_t sl_table_size(sl_table_t* table);
extern int sl_table_exsits(sl_table_t* table, const char* key);
extern void sl_table_set(sl_table_t* table, 
    const char* key, void* value, void (*release)(void*));
extern void* sl_table_get(sl_table_t* table, const char* key);
extern void sl_table_remove(sl_table_t* table, const char* key);

/*
 * @attention:
 *    You should careful to use this interface, 
 *    it may cost your more time O(m * n);
 */
extern void sl_table_traverse(sl_table_t* table, 
    void (*visit)(const char*, void*));

#endif  /* __SL_TABLE_HEADER_H__ */
