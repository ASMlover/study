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
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ru_asm.h"
#include "ru_expr.h"
#include "ru_parser.h"

struct {
  TVariable var[0xFF];
  int count;
} globalVar;
static TVariable localVar[0xFF][0xFF];
static int localVarSize[0xFF];
static int localVarCounter;
static int funcNumber;

struct {
  char* text[0xFF];
  int* addr;
  int count;
} gStrings;

struct {
  TFunc func[0xFF];
  int count;
  int inside;
} gFunctions;

#define NON 0

static TVariable* _AppendVar(const char* name, int type) {
  if (gFunctions.inside == IN_FUNC) {
    int32_t sz = 1 + (++localVarSize[funcNumber]);
    strcpy(localVar[funcNumber][localVarCounter].name, name);
    localVar[funcNumber][localVarCounter].type = type;
    localVar[funcNumber][localVarCounter].id = sz;
    localVar[funcNumber][localVarCounter].loctype = V_LOCAL;

    return &localVar[funcNumber][localVarCounter++];
  }
  else if (gFunctions.inside == IN_GLOBAL) {
    strcpy(globalVar.var[globalVar.count].name, name);
    globalVar.var[globalVar.count].type = type;
    globalVar.var[globalVar.count].loctype = V_GLOBAL;
    globalVar.var[globalVar.count].id = (uint32_t)&gRuCode[gRuCount];
    gRuCount += sizeof(int32_t);

    return &globalVar.var[globalVar.count++];
  }

  return NULL;
}

static TFunc* _AppendFunc(const char* name, int address, int args) {
  gFunctions.func[gFunctions.count].address = address;
  gFunctions.func[gFunctions.count].args = args;
  strcpy(gFunctions.func[gFunctions.count].name, name);
  return &gFunctions.func[gFunctions.count++];
}

static int32_t _AppendBreak(void) {
  ru_Emit(0xe9); /* jump */
  gBreaks.addr = (uint32_t*)realloc(gBreaks.addr, 4 * (gBreaks.count + 1));
  gBreaks.addr[gBreaks.count] = gRuCount;
  ru_EmitI32(0);
  return gBreaks.count++;
}

static int32_t _AppendReturn(void) {
  ru_RelExpr();
  ru_Emit(0xe9); /* jump */
  gReturns.addr = (uint32_t*)realloc(gReturns.addr, 4 * (gReturns.count + 1));
  if (NULL == gReturns.addr)
    ru_Error("No enough memory");
  gReturns.addr[gReturns.count] = gRuCount;
  ru_EmitI32(0);
  return gReturns.count++;
}

int32_t ru_Skip(const char* s) {
  if (!strcmp(s, gToken.token[gToken.pos].value)) {
    ++gToken.pos;
    return 1;
  }
  return 0;
}

int32_t ru_Error(const char* fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);

  exit(0);
  return 0;
}

static int _Eval(int pos, int status) {
  while (gToken.pos < gToken.size) {
    if (ru_Expr(pos, status))
      return 1;
  }

  return 0;
}

static TVariable* _DeclareVar(void) {
  return NULL;
}

static int _IfStmt(void) {
  uint32_t end;

  ru_RelExpr(); /* if condition */
  ru_Emit(0x83); ru_Emit(0xf8); ru_Emit(0x00); /* cmp eax, 0 */
  ru_Emit(0x75); ru_Emit(0x05); /* jne 5 */
  ru_Emit(0xe9); /* jmp */
  end = gRuCount;
  ru_EmitI32(0);

  return _Eval(end, NON);
}

static int _WhileStmt(void) {
  uint32_t loopBg = gRuCount;
  uint32_t end;
  uint32_t stepBg[2];
  uint32_t stepOn = 0;

  ru_RelExpr();
  if (ru_Skip(",")) {
    stepOn = 1;
    stepBg[0] = gToken.pos;
    for (; gToken.token[gToken.pos].value[0] != ';'; ++gToken.pos) {
    }
  }
  ru_Emit(0x83); ru_Emit(0xf8); ru_Emit(0x00); /* cmp eax, 0 */
  ru_Emit(0x75); ru_Emit(0x05); /* jne 5 */
  ru_Emit(0xe9); end = gRuCount; ru_Emit(0); /* jmp while end */

  if (ru_Skip(":"))
    ru_Expr(0, BLOCK_LOOP);
  else
    _Eval(0, BLOCK_LOOP);

  if (stepOn) {
    stepBg[1] = gToken.pos;
    gToken.pos = stepBg[0];
    if (ru_IsAssign())
      ru_Assignment();
    gToken.pos = stepBg[1];
  }

  ru_Emit(0xe9); ru_EmitI32(0xFFFFFFFF - gRuCount + loopBg - 4); /* jmp n */
  ru_EmitI32Insert(gRuCount - end - 4, end);

  for (--gBreaks.count; gBreaks.count >= 0; --gBreaks.count) {
    ru_EmitI32Insert(
        gRuCount - gBreaks.addr[gBreaks.count] - 4,
        gBreaks.addr[gBreaks.count]);
  }
  gBreaks.count = 0;

  return 0;
}

static int32_t _FunctionStmt(void) {
  int32_t escapeBg;
  int32_t nargs = 0;
  const char* funcName = gToken.token[gToken.pos++].value;
  int argPos[128];
  int i;

  if (ru_Skip("(")) {
    do {
      _DeclareVar();
      ++gToken.pos;
      ++nargs;
    } while (ru_Skip(","));
    if (!ru_Skip(")"))
      ru_Error("%d: expecting `)`", gToken.token[gToken.pos].lineno);
  }
  _AppendFunc(funcName, gRuCount, nargs);
  ru_Emit(0x50 + RU_EBP); /* push ebp */
  ru_Emit(0x89); ru_Emit(0xc0 + RU_ESP * 8 + RU_EBP); /* mov evp esp */
  escapeBg = gRuCount + 2;
  /* sub esp 0; align */
  ru_Emit(0x81); ru_Emit(0xe8 + RU_ESP); ru_EmitI32(0);

  for (i = 0; i < nargs; ++i) {
    ru_Emit(0x8b);
    ru_Emit(0x45);
    ru_Emit(0x08 + (nargs - i - 1) * sizeof(int32_t));

    ru_Emit(0x89);
    ru_Emit(0x44);
    ru_Emit(0x24);

    argPos[i] = gRuCount;
    ru_Emit(0x00);
  }
  _Eval(0, BLOCK_LOOP);

  for (--gReturns.count; gReturns.count >= 0; --gReturns.count) {
    ru_EmitI32Insert(
        gRuCount - gReturns.addr[gReturns.count] - 4,
        gReturns.addr[gReturns.count]);
  }
  gReturns.count = 0;

  ru_Emit(0x81); ru_Emit(0xc0 + RU_ESP);
  /* add esp nn */
  ru_EmitI32(sizeof(int32_t) * (localVarSize[funcNumber] + 6));
  ru_Emit(0xc9); /* leave */
  ru_Emit(0xc3); /* ret */

  ru_EmitI32Insert(
      sizeof(int32_t) * (localVarSize[funcNumber] + 6), escapeBg);
  for (i = 1; i <= nargs; ++i) {
    gRuCode[argPos[i - 1]] = 256 - sizeof(int32_t) * i +
      (((localVarSize[funcNumber] + 6) * sizeof(int32_t)) - 4);
  }

  return 0;
}

static const char* _ReplaceEscape(const char* str) {
  char escape[12][3] = {
    "\\a", "\a", "\\r", "\r", "\\f", "\f",
    "\\n", "\n", "\\t", "\t", "\\b", "\b",
  };
  int i;

  for (i = 0; i < 12; i += 2) {
    char* pos;
    while ((pos = strstr((char*)str, escape[i])) != NULL) {
      *pos = escape[i + 1][0];
      memmove(pos + 1, pos + 2, strlen(pos + 2) + 1);
    }
  }

  return str;
}

int ru_Expr(int pos, int status) {
  return 0;
}

int ru_GetString(void) {
  gStrings.text[gStrings.count] =
    (char*)calloc(sizeof(char),
        strlen(gToken.token[gToken.pos].value) + 1);
  strcpy(gStrings.text[gStrings.count], gToken.token[gToken.pos++].value);

  *gStrings.addr++ = gRuCount;
  return gStrings.count++;
}

TVariable* ru_GetVar(const char* name) {
  int i;

  /* local variables */
  for (i = 0; i < localVarCounter; ++i) {
    if (!strcmp(name, localVar[funcNumber][i].name))
      return &localVar[funcNumber][i];
  }

  /* global variables */
  for (i = 0; i < globalVar.count; ++i) {
    if (!strcmp(name, globalVar.var[i].name))
      return &globalVar.var[i];
  }

  return NULL;
}

TFunc* ru_GetFunc(const char* name) {
  int i;
  for (i = 0; i < gFunctions.count; ++i) {
    if (!strcmp(gFunctions.func[i].name, name))
      return &gFunctions.func[i];
  }

  return NULL;
}

int ru_IsAssign(void) {
  const char* value = gToken.token[gToken.pos + 1].value;
  if (!strcmp(value, "=") || !strcmp(value, "++") || !strcmp(value, "--"))
    return 1;
  if (!strcmp(value, "[")) {
    int32_t i = gToken.pos + 2;
    int32_t t = 1;
    while (t) {
      value = gToken.token[i].value;
      if (!strcmp(value, "["))
        ++t;
      if (!strcmp(value, "]"))
        --t;
      if (!strcmp(value, ";")) {
        ru_Error("%d: invalid expression",
            gToken.token[gToken.pos].lineno);
      }
      ++i;
    }
    if (!strcmp(gToken.token[i].value, "="))
      return 1;
  }
  else if (!strcmp(value, ":")
      && !strcmp(gToken.token[gToken.pos + 3].value, "=")) {
    return 1;
  }

  return 0;
}

int ru_Assignment(void) {
  return 0;
}

int ru_Parser(void) {
  uint32_t mainAddress;
  uint32_t addr;
  int i;

  gToken.pos = gRuCount = 0;
  gStrings.addr = (int*)calloc(0xff, sizeof(int32_t));
  ru_Emit(0xe9);
  mainAddress = gRuCount;
  ru_EmitI32(0);
  _Eval(0, 0);

  addr = ru_GetFunc("main")->address;
  ru_EmitI32Insert(addr - 5, mainAddress);

  for (gStrings.addr--; gStrings.count; --gStrings.addr) {
    ru_EmitI32Insert((uint32_t)&gRuCode[gRuCount], *gStrings.addr);
    _ReplaceEscape(gStrings.text[--gStrings.count]);
    for (i = 0; gStrings.text[gStrings.count][i]; ++i)
      ru_Emit(gStrings.text[gStrings.count][i]);
    ru_Emit(0); /* '\0' */
  }

  return 1;
}
