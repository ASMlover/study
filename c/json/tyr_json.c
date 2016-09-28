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
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "tyr_json.h"

#define EXPECT(c, ch) do {\
  assert(*c->json == (ch));\
  c->json++;\
} while (0)

typedef struct tyr_context {
  const char* json;
} tyr_context;

#define TYR_PARSE_IDENTITY(c, id, n)\
  EXPECT(c, id[0]);\
  if (0 != strncmp(c->json, id, (n)))\
    return TYR_PARSE_INVALID_VALUE;\
  c->json += (n) - 1

static void tyr_parse_whitespace(tyr_context* c) {
  const char* p = c->json;
  while (isspace(*p))
    ++p;
  c->json = p;
}

static int tyr_parse_null(tyr_context* c, tyr_value* value) {
  TYR_PARSE_IDENTITY(c, "null", 4);
  value->type = TYR_NULL;
  return TYR_PARSE_OK;
}

static int tyr_parse_true(tyr_context* c, tyr_value* value) {
  TYR_PARSE_IDENTITY(c, "true", 4);
  value->type = TYR_TRUE;
  return TYR_PARSE_OK;
}

static int tyr_parse_false(tyr_context* c, tyr_value* value) {
  TYR_PARSE_IDENTITY(c, "false", 5);
  value->type = TYR_FALSE;
  return TYR_PARSE_OK;
}

static int tyr_parse_number(tyr_context* c, tyr_value* value) {
  char* end;
  value->u.number = strtod(c->json, &end);
  if (c->json == end)
    return TYR_PARSE_INVALID_VALUE;
  c->json = end;
  value->type = TYR_NUMBER;
  return TYR_PARSE_OK;
}

static int tyr_parse_value(tyr_context* c, tyr_value* value) {
  switch (*c->json) {
  case 'n':
    return tyr_parse_null(c, value);
  case 't':
    return tyr_parse_true(c, value);
  case 'f':
    return tyr_parse_false(c, value);
  case '\0':
    return TYR_PARSE_EXPECT_VALUE;
  default:
    return tyr_parse_number(c, value);
  }
}

int tyr_parse(tyr_value* value, const char* json) {
  tyr_context c;
  int r;
  assert(NULL != value);

  c.json = json;
  value->type = TYR_NULL;
  tyr_parse_whitespace(&c);
  if (TYR_PARSE_OK == (r = tyr_parse_value(&c, value))) {
    tyr_parse_whitespace(&c);
    if ('\0' != *c.json) {
      value->type = TYR_NULL;
      r = TYR_PARSE_ROOT_NOT_SINGULAR;
    }
  }
  return r;
}

void tyr_free(tyr_value* value) {
  assert(NULL != value);
  if (TYR_STRING == value->type)
    free(value->u.string.s);
  value->type = TYR_NULL;
}

tyr_type tyr_get_type(const tyr_value* value) {
  assert(NULL != value);
  return value->type;
}

int tyr_get_boolean(const tyr_value* value) {
  assert(NULL != value);
  /* TODO: */
  return 0;
}

void tyr_set_boolean(tyr_value* value, int b) {
  assert(NULL != value);
  /* TODO: */
}

double tyr_get_number(const tyr_value* value) {
  assert(NULL != value && TYR_NUMBER == value->type);
  return value->u.number;
}

void tyr_set_number(tyr_value* value, double n) {
  assert(NULL != value);
  /* TODO: */
}

const char* tyr_get_string(const tyr_value* value) {
  assert(NULL != value && TYR_STRING == value->type);
  return value->u.string.s;
}

size_t tyr_get_string_length(const tyr_value* value) {
  assert(NULL != value && TYR_STRING == value->type);
  return value->u.string.n;
}

void tyr_set_string(tyr_value* value, const char* s, size_t n) {
  assert(NULL != value && (NULL != s || 0 == n));
  tyr_free(value);
  value->u.string.s = (char*)malloc(n + 1);
  memcpy(value->u.string.s, s, n);
  value->u.string.s[n] = 0;
  value->u.string.n = n;
  value->type = TYR_STRING;
}
