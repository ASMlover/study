/*
 * Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_JSON_HEADER_H__
#define __TYR_JSON_HEADER_H__

#include <stddef.h>

typedef enum {
  TYR_NULL,
  TYR_FALSE,
  TYR_TRUE,
  TYR_NUMBER,
  TYR_STRING,
  TYR_ARRAY,
  TYR_OBJECT,
} tyr_type;

typedef struct tyr_value tyr_value;
typedef struct tyr_member tyr_member;

struct tyr_value {
  union {
    struct { tyr_member* m; size_t n; } object;
    struct { tyr_value* e; size_t n; } array;
    struct { char* s; size_t n; } string;
    double number;
  } u;
  tyr_type type;
};

struct tyr_member {
  char* k; size_t klen; /* key of member */
  tyr_value v;
};

enum {
  TYR_PARSE_OK = 0,
  TYR_PARSE_EXPECT_VALUE,
  TYR_PARSE_INVALID_VALUE,
  TYR_PARSE_ROOT_NOT_SINGULAR,
  TYR_PARSE_NUMBER_TO_BIG,
  TYR_PARSE_MISS_QUOTATION_MARK,
  TYR_PARSE_INVALID_STRING_ESCAPE,
  TYR_PARSE_INVALID_STRING_CHAR,
  TYR_PARSE_INVALID_UNICODE_HEX,
  TYR_PARSE_INVALID_UNICODE_SURROGATE,
  TYR_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
  TYR_PARSE_MISS_KEY,
  TYR_PARSE_MISS_COLON,
  TYR_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
};

#define tyr_init(v) do { (v)->type = TYR_NULL; } while (0)
#define tyr_set_nil(v) tyr_free(v)

int tyr_parse(tyr_value* value, const char* json);
char* tyr_stringify(const tyr_value* value, size_t* length);
void tyr_free(tyr_value* value);
tyr_type tyr_get_type(const tyr_value* value);

int tyr_get_boolean(const tyr_value* value);
void tyr_set_boolean(tyr_value* value, int b);
double tyr_get_number(const tyr_value* value);
void tyr_set_number(tyr_value* value, double n);
const char* tyr_get_string(const tyr_value* value);
size_t tyr_get_string_length(const tyr_value* value);
void tyr_set_string(tyr_value* value, const char* s, size_t n);
size_t tyr_get_array_size(const tyr_value* value);
tyr_value* tyr_get_array_element(const tyr_value* value, size_t index);
size_t tyr_get_object_size(const tyr_value* value);
const char* tyr_get_object_key(const tyr_value* value, size_t index);
size_t tyr_get_object_key_length(const tyr_value* value, size_t index);
tyr_value* tyr_get_object_value(const tyr_value* value, size_t index);

#endif /* __TYR_JSON_HEADER_H__ */
