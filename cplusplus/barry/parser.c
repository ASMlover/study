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

static int _ParseNode(BarryNode* node);

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
      TOKEN_LPAREN != node->next->token.type)
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

static void*
_GetDeclaration(BarryNode* node)
{
  const char* key = node->token.as.string;
  int i = 0;

#define VAR SCOPE->decls[i]
#define FIND()\
  for (i = 0; i < SCOPE->declLen; ++i) {\
    if (EQUAL(key, VAR.key))\
      return VAR.value;\
  }

#define SCOPE node->scope
  FIND();
#undef SCOPE

#define SCOPE BARRY_GLOBAL
  FIND();
#undef SCOPE

#undef VAR
#undef FIND

  return NULL;
}

static BarryDef*
_GetDefinition(BarryNode* node)
{
  const char* name = node->token.as.string;
  int i = 0;

#define DEF SCOPE->definitions[i]
#define FIND()\
  for (i = 0; i < SCOPE->definitionLen; ++i) {\
    if (EQUAL(name, DEF.name))\
      return &DEF;\
  }

#define SCOPE node->scope
  FIND();
#undef SCOPE

#define SCOPE BARRY_GLOBAL
  FIND();
#undef SCOPE

#undef DEF
#undef FIND

  return NULL;
}

static int
_AssignDeclaraionNode(BarryNode* node)
{
  BarryNode* value = node->next->next;
  const char* key = node->token.as.string;
  void* val = NULL;

  if (NULL == value)
    return 1;

  switch (value->token.type) {
  case TOKEN_ID:
    val = _GetDeclaration(value);
    barry_Declartion(key, val);
    break;
  case TOKEN_NUMBER:
  case TOKEN_STR:
    val = value->token.as.string;
    barry_Declartion(key, val);
    break;
  }
  node->ast->current = value->next;

  return 0;
}

static int
_AssignFunctionDeclarationNode(BarryNode* node)
{
  BarryNode* next = node->next;
  const char* name = NULL;
  char body[BUFSIZ];
  int rbrace = 0;

  name = next->token.as.string;

  next = next->next;
  while (TOKEN_END != next->token.type || rbrace > 0) {
    if (TOKEN_STR == next->token.type)
      sprintf(body, "\"%s\"", next->token.as.string);
    else
      sprintf(body, "%s", next->token.as.string);

    next = next->next;
    if (TOKEN_LBRACE == next->token.type) {
      ++rbrace;
      sprintf(body, "%s", next->token.as.string);
      next = next->next;
    }
    if (TOKEN_RBRACE == next->token.type && rbrace > 0) {
      --rbrace;
      sprintf(body, "%s", next->token.as.string);
      next = next->next;
    }
  }

  barry_Definition(name, strdup(body));
  node->ast->current = next->next;

  return 0;
}

static int
_CallFunctionNode(BarryNode* node)
{
  BarryFunArguments arguments;
  BarryNode* next = node->next;

  arguments.length = 0;
  if (TOKEN_LPAREN != next->token.type) {
    ERROR("Unexpected token. Expected `(`\n");
    return 1;
  }

  next = node->next;
  if (NULL == next) {
    ERROR("Unexpected end of input\n");
    return 1;
  }

#define PUSH(v) arguments.values[arguments.length++] = v
  while (TOKEN_RPAREN != next->token.type) {
    node->ast->current = next;
    next = next->next;
    switch (next->token.type) {
    case TOKEN_RPAREN:
    case TOKEN_COMMA:
      break;
    case TOKEN_NUMBER:
    case TOKEN_STR:
      PUSH(next->token.as.string);
      break;
    case TOKEN_ID:
      _ParseNode(next);
      if (_IsFunction(next)) {
        if (_CallFunctionNode(node) > 0)
          return 1;
        break;
      }
      if (_IsDeclaration(next)) {
        PUSH(_GetDeclaration(next));
      }
      else {
        ERROR("Error [%d:%d]: `%s` is not defined\n",
            next->token.lineno,
            (next->prev->token.colno - strlen(next->token.as.string)),
            next->token.as.string);
        return 1;
      }
      break;
    default:
      ERROR("Error: Unexpected token `%s`", next->token.as.string);
      return 1;
    }
  }
#undef PUSH

  node->ast->current = next->next;
  BarryFunction* fn = _GetFunction(node);
  BarryDef* def = NULL;

  if (NULL == fn) {
    def = _GetDefinition(node);
    if (NULL == def)
      ERROR("%s is not a function", node->token.as.string);
    else
      return barry_Parse((char*)def->name, (char*)def->body, def->locals);
    return 1;
  }
  else {
    fn->function(arguments);
  }

  return 0;
}

static int
_ParseLoop(BarryNode* node)
{
  return 0;
}

static int
_ParseIf(BarryNode* node)
{
  if (TOKEN_IF != node->token.type) {
    ERROR("Error: Unexpected token `%s`, expected `if`",
        node->token.as.string);
    return 1;
  }

  if (TOKEN_RPAREN != node->token.type) {
    ERROR("Error: Unexpected token `%s`, expected `(`",
        node->token.as.string);
    return 1;
  }

  return 0;
}

static int
_ParseNode(BarryNode* node)
{
  node->ast->current = node->next;
  switch (node->token.type) {
  case TOKEN_NONE:
    break;
  case TOKEN_ID:
    if (_IsFunction(node)) {
      if (_CallFunctionNode(node) > 0)
        return 1;
      break;
    }
    if (_IsAssignment(node)) {
      if (_AssignDeclaraionNode(node) > 0)
        return 1;
      break;
    }
    if (_IsFunctionDeclaration(node)) {
      if (_AssignFunctionDeclarationNode(node) > 0)
        return 1;
    }
    break;
  case TOKEN_FOR:
    if (_ParseLoop(node) > 0)
      return 1;
    break;
  case TOKEN_IF:
    if (_ParseIf(node) > 0)
      return 1;
    break;
  case TOKEN_ELSE:
    break;
  }

  return 0;
}

int
barry_Eval(BarryAST* ast)
{
  ast->current = ast->nodes[0];
  while (ast->current) {
    if (_ParseNode(ast->current) > 0)
      return 1;
  }
  return 0;
}

int
barry_Parse(const char* file, const char* src, BarryScope* scope)
{
  BarryLexer* lexer = (BarryLexer*)malloc(sizeof(BarryLexer));
  BarryNode main;
  BarryAST* ast = (BarryAST*)malloc(sizeof(BarryAST));
  BarryDef* def = NULL;
  int r = 0;

  if (NULL == lexer) {
    ERROR("Error: lexer allocation error\n");
    return 1;
  }
  if (NULL == ast) {
    ERROR("Error: ast allocation error\n");
    return 1;
  }

  lexer->src = (char*)src;
  lexer->file = (char*)file;
  lexer->colno = 0;
  lexer->lineno = 1;
  lexer->offset = 0;
  lexer->curr.type = TOKEN_NONE;
  lexer->curr.as.string = (char*)"";

  ast->file = (char*)file;

  while (0 == barry_LexerScan(lexer)) {
    BarryNode* node = (BarryNode*)malloc(sizeof(BarryNode));
    node->token = lexer->curr;
    node->ast = ast;
    node->scope = NULL == scope ? BARRY_GLOBAL : scope;
    if (ast->length > 0)
      node->prev = ast->nodes[ast->length - 1]->next = node;
    ast->nodes[ast->length++] = node;
  }
  free(lexer);

  r = barry_Eval(ast);
  if (NULL == scope) {
    main.ast = ast;
    main.token.type = TOKEN_ID;
    main.token.as.string = (char*)"main";
    main.scope = BARRY_GLOBAL;

    def = _GetDefinition(&main);
    if (NULL != def) {
      r = barry_Parse((const char*)def->name,
          (const char*)def->body, def->locals);
    }
  }

  return r;
}
