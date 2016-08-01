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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "common.h"
#include "global.h"

BarryScope* BARRY_GLOBAL;

static void*
_barry_Print(BarryFunArguments arguments)
{
  int i = 0;
  int size = arguments.length;

  for (; i < size; ++i) {
    char* value = (char*)arguments.values[i];
    char out[BUFSIZ];
    {
      char ch = 0;
      char p = 0;
      int x = 0;
      int len = (int)strlen(value);
      size_t s = 0;
      for (; x < len; ++x) {
        ch = value[x];
        p = value[x + 1];
        if ('\\' == ch && '"' == p) {
          out[s++] = p;
          ++x;
        }
        else {
          out[s++] = ch;
        }
      }

      out[s++] = '\0';
    }

    fprintf(stdout, "%s", out);
    if (i < size - 1)
      fprintf(stdout, " ");
  }
  fprintf(stdout, "\n");

  return NULL;
}

void
barry_InitGlobals(void)
{
  if (NULL == BARRY_GLOBAL) {
    BARRY_GLOBAL = (BarryScope*)malloc(sizeof(BarryScope));
    if (NULL == BARRY_GLOBAL) {
      ERROR("Memory allocation error!");
      exit(1);
    }
  }

  BARRY_GLOBAL->functionLen = 0;
  BARRY_GLOBAL->declLen = 0;
  BARRY_BUILTIN_FUN("print", _barry_Print);
}

void
barry_Declartion(const char* key, void* value)
{
  int i = 0;
  int len = (int)BARRY_GLOBAL->declLen;

  for (; i < len; ++i) {
    if (EQUAL(key, BARRY_GLOBAL->decls[i].key)) {
      BARRY_GLOBAL->decls[i].value = value;
      break;
    }
  }

  BARRY_GLOBAL->decls[BARRY_GLOBAL->declLen++] =
    (BarryDeclaration){.key = key, .value = value};
}

void
barry_Definition(const char* name, const char* body)
{
  int i = 0;
  int len = (int)BARRY_GLOBAL->definitionLen;
  for (; i < len; ++i) {
    if (EQUAL(name, BARRY_GLOBAL->definitions[i].name)) {
      BARRY_GLOBAL->definitions[i].body = body;
      break;
    }
  }

  BARRY_GLOBAL->definitions[BARRY_GLOBAL->definitionLen++] =
    (BarryDef) {
      .name = name,
      .body = body,
      .locals = (BarryScope*)malloc(sizeof(BarryScope)),
      .globals = BARRY_GLOBAL
    };
}
