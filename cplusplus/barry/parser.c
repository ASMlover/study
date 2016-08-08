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
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "global.h"
#include "ast.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"

static char*
_Ftoa(float num)
{
  int i = 0;
  char str[80];
  int m = log10(num);
  int digit;
  float tolerance = 0.0001f;
  float precision = 0.00000000000001;

  while (num > 0 + precision) {
    float weight = pow(10.0f, m);
    digit = floor(num / weight);
    num -= (digit * weight);
    str[i++] = '0' + digit;
    if (0 == m)
      str[i++] = '.';
    --m;
  }
  str[i] = '\0';

  return strdup(str);
}

static int
_IsFunction(BarryNode* node)
{
  if (TOKEN_ID != node->token.type ||
      TOKEN_LPARAM != node->next->token.type)
    return 0;

  return 1;
}

static int
_IsFunctionDeclaration(BarryNode* node)
{
  BarryNode* next = node->next;
  char* name = NULL;

  if (TOKEN_ID != node->token.type)
    return 0;
  if (!EQUAL("def", node->token.as.string))
    return 0;
  if (NULL == node->next) {
    ERROR("Unexpected end of input");
    return 0;
  }

  name = next->token.as.string;
  next = next->next;

  if (NULL == name) {
    ERROR("Unexpected token. Expected name.");
    return 0;
  }
  if (NULL == next) {
    ERROR("Unexpected end of input");
    return 0;
  }

  next = next->next;
  if (NULL == next) {
    ERROR("Unexpected end of input");
    return 0;
  }

  return 1;
}

static int
_IsAssignment(BarryNode* node)
{
  return (TOKEN_ASSIGN == node->next->token.type);
}

static int
_IsDeclaration(BarryNode* node)
{
  int i = 0;
  if (TOKEN_ID != node->token.type)
    return 0;

#define VAR SCOPE->decls[i]
#define FIND()\
  for (i = 0; i < SCOPE->declLen; ++i) {\
    if (EQUAL(node->token.as.string, VAR.key))\
      return 1;\
  }

#define SCOPE node->scope
  FIND();
#undef SCOPE

#define SCOPE BARRY_GLOBAL
  FIND();
#undef SCOPE

#undef VAR
#undef FIND

  return 0;
}

static BarryFunction*
_GetFunction(BarryNode* node)
{
  const char* name = node->token.as.string;
  int i = 0;

#define FN SCOPE->functions[i]
#define FIND()\
  for (i = 0; i < SCOPE->functionLen; ++i) {\
    if (EQUAL(name, FN.name))\
      return &FN;\
  }

#define SCOPE node->scope
  FIND();
#undef SCOPE

#define SCOPE BARRY_GLOBAL
  FIND()
#undef SCOPE

#undef FN
#undef FIND

  return NULL;
}
