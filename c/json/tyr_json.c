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

static void tyr_parse_whitespace(tyr_context* c) {
  const char* p = c->json;
  while (isspace(*p))
    ++p;
  c->json = p;
}

static int tyr_parse_null(tyr_context* c, tyr_value* value) {
  EXPECT(c, 'n');
  if (0 != strncmp(c->json, "null", 4))
    return TYR_PARSE_INVALID_VALUE;
  c->json += 3;
  value->type = TYR_NULL;
  return TYR_PARSE_OK;
}

static int tyr_parse_value(tyr_context* c, tyr_value* value) {
  switch (*c->json) {
  case 'n':
    return tyr_parse_null(c, value);
  case '\0':
    return TYR_PARSE_EXPECT_VALUE;
  default:
    return TYR_PARSE_INVALID_VALUE;
  }
}

int tyr_parse(tyr_value* value, const char* json) {
  tyr_context c;
  assert(NULL != value);

  c.json = json;
  value->type = TYR_NULL;
  tyr_parse_whitespace(&c);

  return tyr_parse_value(&c, value);
}

tyr_type tyr_get_type(const tyr_value* value) {
  assert(NULL != value);
  return value->type;
}
