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

typedef enum {
  TYR_NULL,
  TYR_FALSE,
  TYR_TRUE,
  TYR_NUMBER,
  TYR_STRING,
  TYR_ARRAY,
  TYR_OBJECT,
} tyr_type;

typedef struct tyr_value {
  double number;
  tyr_type type;
} tyr_value;

enum {
  TYR_PARSE_OK = 0,
  TYR_PARSE_EXPECT_VALUE,
  TYR_PARSE_INVALID_VALUE,
  TYR_PARSE_ROOT_NOT_SINGULAR,
};

int tyr_parse(tyr_value* value, const char* json);
tyr_type tyr_get_type(const tyr_value* value);
double tyr_get_number(const tyr_value* value);

#endif /* __TYR_JSON_HEADER_H__ */
