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
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "tyr_json.h"

#define TYR_PARSE_INIT_STACKSZ 256

#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define EXPECT(c, ch) do {\
  assert(*c->json == (ch));\
  c->json++;\
} while (0)
#define PUTC(c, ch) do {\
  *(char*)tyr_context_push(c, sizeof(char)) = (ch);\
} while (0)

typedef struct tyr_context {
  const char* json;
  char* stack;
  size_t size;
  size_t top;
} tyr_context;

#define TYR_PARSE_IDENTITY(c, id, n)\
  EXPECT(c, id[0]);\
  if (0 != strncmp(c->json, id, (n)))\
    return TYR_PARSE_INVALID_VALUE;\
  c->json += (n) - 1

#define tyr_context_init(c, json) do {\
  c.json = json;\
  c.stack = NULL;\
  c.size = 0;\
  c.top = 0;\
} while (0)
#define tyr_context_destroy(c) do {\
  if (NULL != c.stack)\
    free(c.stack);\
} while (0)

static void* tyr_context_push(tyr_context* c, size_t size) {
  void* ret;

  assert(size > 0);
  if (c->top + size >= c->size) {
    if (0 == c->size)
      c->size = TYR_PARSE_INIT_STACKSZ;
    while (c->top + size >= c->size)
      c->size += c->size >> 1;
    c->stack = (char*)realloc(c->stack, c->size);
  }
  ret = c->stack + c->top;
  c->top += size;

  return ret;
}

static void* tyr_context_pop(tyr_context* c, size_t size) {
  assert(c->top >= size);
  return c->stack + (c->top -= size);
}

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
  const char* p = c->json;

  if ('-' == *p)
    ++p;

  if ('0' == *p) {
    ++p;
  }
  else {
    if (!isdigit(*p))
      return TYR_PARSE_INVALID_VALUE;
    for (p++; isdigit(*p); ++p) {}
  }

  if ('.' == *p) {
    ++p;
    if (!isdigit(*p))
      return TYR_PARSE_INVALID_VALUE;
    for (p++; isdigit(*p); ++p) {}
  }

  if ('e' == *p || 'E' == *p) {
    ++p;
    if ('+' == *p || '-' == *p)
      ++p;
    if (!isdigit(*p))
      return TYR_PARSE_INVALID_VALUE;
    for (p++; isdigit(*p); ++p) {}
  }

  errno = 0;
  value->u.number = strtod(c->json, NULL);
  if (ERANGE == errno
      && (HUGE_VAL == value->u.number || -HUGE_VAL == value->u.number))
    return TYR_PARSE_NUMBER_TO_BIG;
  value->type = TYR_NUMBER;
  c->json = p;
  return TYR_PARSE_OK;
}

static int tyr_parse_string(tyr_context* c, tyr_value* value) {
  size_t head = c->top;
  size_t len;
  const char* p;

  EXPECT(c, '\"');
  p = c->json;
  for (;;) {
    char ch = *p++;
    switch (ch) {
    case '\"':
      len = c->top - head;
      tyr_set_string(value, (const char*)tyr_context_pop(c, len), len);
      c->json = p;
      return TYR_PARSE_OK;
    case '\\':
      switch (*p++) {
      case '\"': PUTC(c, '\"'); break;
      case '\\': PUTC(c, '\\'); break;
      case '/': PUTC(c, '/'); break;
      case 'b': PUTC(c, '\b'); break;
      case 'f': PUTC(c, '\f'); break;
      case 'r': PUTC(c, '\r'); break;
      case 'n': PUTC(c, '\n'); break;
      case 't': PUTC(c, '\t'); break;
      default:
        c->top = head;
        return TYR_PARSE_INVALID_STRING_ESCAPE;
      }
      break;
    case '\0':
      c->top = head;
      return TYR_PARSE_MISS_QUOTATION_MARK;
    default:
      if ((unsigned char)ch < 0x20) {
        c->top = head;
        return TYR_PARSE_INVALID_STRING_CHAR;
      }
      PUTC(c, ch);
    }
  }
}

static int tyr_parse_value(tyr_context* c, tyr_value* value) {
  switch (*c->json) {
  case 'n': return tyr_parse_null(c, value);
  case 't': return tyr_parse_true(c, value);
  case 'f': return tyr_parse_false(c, value);
  case '"': return tyr_parse_string(c, value);
  case '\0': return TYR_PARSE_EXPECT_VALUE;
  default: return tyr_parse_number(c, value);
  }
}

int tyr_parse(tyr_value* value, const char* json) {
  tyr_context c;
  int r;
  assert(NULL != value);

  tyr_context_init(c, json);
  tyr_init(value);
  tyr_parse_whitespace(&c);
  if (TYR_PARSE_OK == (r = tyr_parse_value(&c, value))) {
    tyr_parse_whitespace(&c);
    if ('\0' != *c.json) {
      value->type = TYR_NULL;
      r = TYR_PARSE_ROOT_NOT_SINGULAR;
    }
  }

  assert(0 == c.top);
  tyr_context_destroy(c);
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
  assert(NULL != value
      && (TYR_TRUE == value->type || TYR_FALSE == value->type));
  return value->type == TYR_TRUE;
}

void tyr_set_boolean(tyr_value* value, int b) {
  assert(NULL != value);
  tyr_free(value);
  value->type = b ? TYR_TRUE : TYR_FALSE;
}

double tyr_get_number(const tyr_value* value) {
  assert(NULL != value && TYR_NUMBER == value->type);
  return value->u.number;
}

void tyr_set_number(tyr_value* value, double n) {
  assert(NULL != value);
  tyr_free(value);
  value->u.number = n;
  value->type = TYR_NUMBER;
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
