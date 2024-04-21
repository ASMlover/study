/*
 * Copyright (c) 2024 ASMlover. All rights reserved.
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
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "scanner.h"

typedef struct {
  Token                 current;
  Token                 previous;
  bool                  hadError;
  bool                  panicMode;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,      // =
  PREC_OR,              // or
  PREC_AND,             // and
  PREC_EQUALITY,        // == !=
  PREC_COMPARISON,      // < > <= >=
  PREC_TERM,            // + -
  PERC_FACTOR,          // * /
  PREC_UNARY,           // ! -
  PREC_CALL,            // . ()
  PREC_PRIMARY,
} Precedence;

typedef void (*ParseFn)(bool canAssign);
typedef struct {
  ParseFn               prefix;
  ParseFn               infix;
  Precedence            precedence;
} ParseRule;

typedef struct {
  Token                 name;
  int                   depth;
  bool                  isCaptured;
} Local;

typedef struct {
  u8_t                  index;
  bool                  isLocal;
} Upvalue;

typedef enum {
  TYPE_FUNCTION,
  TYPE_INITIALIZER,
  TYPE_METHOD,
  TYPE_SCRIPT,
} FunctionType;

typedef struct Compiler {
  struct Compiler*      enclosing;
  ObjFunction*          function;
  FunctionType          type;
  Local                 locals[UINT8_COUNT];
  int                   localCount;
  Upvalue               upvalues[UINT8_COUNT];
  int                   scopeDepth;
} Compiler;

typedef struct ClassCompiler {
  struct ClassCompiler* enclosing;
  bool                  hasSuperclass;
} ClassCompiler;

Parser         parser;
Compiler*      current      = NULL;
ClassCompiler* currentClass = NULL;

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static inline Chunk* currentChunk() {
  return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
  if (parser.panicMode)
    return;

  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->lineno);

  if (TOKEN_EOF == token->type) {
    fprintf(stderr, " at end");
  }
  else if (TOKEN_ERROR == token->type) {
  }
  else {
    fprintf(stderr, " at `%.*s`", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

static void error(const char* message) {
  errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
  errorAt(&parser.current, message);
}

static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (TOKEN_ERROR != parser.current.type)
      break;

    errorAtCurrent(parser.current.start);
  }
}

static void consume(TokenType type, const char* message) {
  if (parser.current.type == type)
    advance();
  else
    errorAtCurrent(message);
}

static inline bool check(TokenType type) {
  return parser.current.type == type;
}

static bool match(TokenType type) {
  if (!check(type))
    return false;

  advance();
  return true;
}

static inline void emitByte(u8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.lineno);
}

static void emitBytes(u8_t byte1, u8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitLoop(int loopStart) {
  emitByte(OP_LOOP);

  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX)
    error("Loop body too large.");

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

static int emitJump(u8_t instruction) {
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count - 2;
}

static void emitReturn() {
  if (TYPE_INITIALIZER == current->type)
    emitBytes(OP_GET_LOCAL, 0);
  else
    emitByte(OP_NIL);

  emitByte(OP_RETURN);
}

static u8_t makeConstant(Value value) {
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (u8_t)constant;
}

static void emitConstant(Value value) {
  emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
  int jump = currentChunk()->count - offset - 2;
  if (jump > UINT16_MAX)
    error("Too match code to jump over.");

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
  compiler->enclosing   = current;
  compiler->function    = NULL;
  compiler->type        = type;
  compiler->localCount  = 0;
  compiler->scopeDepth  = 0;
  compiler->function    = newFunction();

  current = compiler;
  if (TYPE_SCRIPT != type)
    current->function->name = copyString(parser.previous.start, parser.previous.length);

  Local* local = &current->locals[current->localCount++];
  local->depth = 0;
  local->isCaptured = false;

  if (TYPE_FUNCTION != type) {
    local->name.start = "this";
    local->name.length = 4;
  }
  else {
    local->name.start = "";
    local->name.length = 0;
  }
}

static ObjFunction* endCompiler() {
  emitReturn();
  ObjFunction* function = current->function;

#if defined(LOXC_DEBUG_PRINT_CODE)
  if (!parser.hadError)
    disassembleChunk(currentChunk(), NULL != function->name ? function->name->chars : "<script>");
#endif

  current = current->enclosing;
  return function;
}

static void beginScope() {
  ++current->scopeDepth;
}

static void endScope() {
  --current->scopeDepth;

  while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
    if (current->locals[current->localCount - 1].isCaptured)
      emitByte(OP_CLOSE_UPVALUE);
    else
      emitByte(OP_POP);

    --current->localCount;
  }
}

static inline u8_t identifierConstant(Token* name) {
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static inline bool identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length)
    return false;
  return 0 == memcmp(a->start, b->start, a->length);
}

static int resolveLocal(Compiler* compiler, Token* name) {
  for (int i = compiler->localCount - 1; i >= 0; --i) {
    Local* local = &compiler->locals[i];
    if (identifiersEqual(name, &local->name)) {
      if (-1 == local->depth)
        error("Cannot read local variable in its own initializer.");

      return i;
    }
  }
  return -1;
}

static int addUpvalue(Compiler* compiler, u8_t index, bool isLocal) {
  int upvalueCount = compiler->function->upvalueCount;
  for (int i = 0; i < upvalueCount; ++i) {
    Upvalue* Upvalue = &compiler->upvalues[i];
    if (Upvalue->index == index && Upvalue->isLocal == isLocal)
      return i;
  }

  if (UINT8_COUNT == upvalueCount) {
    error("Too many closure variables in function.");
    return 0;
  }

  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;
  return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
  if (NULL == compiler->enclosing)
    return -1;

  int local = resolveLocal(compiler->enclosing, name);
  if (-1 != local) {
    compiler->enclosing->locals[local].isCaptured = true;
    return addUpvalue(compiler, (u8_t)local, true);
  }

  int upvalue = resolveUpvalue(compiler->enclosing, name);
  if (-1 != upvalue)
    return addUpvalue(compiler, (u8_t)upvalue, false);

  return -1;
}

static void addLocal(Token name) {
  if (UINT8_COUNT == current->localCount) {
    error("Too many local variables in function.");
    return;
  }

  Local* local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = -1;
  local->isCaptured = false;
}

static void declareVariable() {
  if (0 == current->scopeDepth)
    return;

  Token* name = &parser.previous;
  for (int i = current->localCount - 1; i >= 0; --i) {
    Local* local = &current->locals[i];
    if (-1 != local->depth && local->depth < current->scopeDepth)
      break;

    if (identifiersEqual(name, &local->name))
      error("Already a variable with this name in this scope.");
  }

  addLocal(*name);
}

static u8_t parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  if (current->scopeDepth > 0)
    return 0;

  return identifierConstant(&parser.previous);
}

static void markInitialized() {
  if (0 == current->scopeDepth)
    return;
  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(u8_t global) {
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

static u8_t argumentList() {
  u8_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();

      if (argCount >= 255)
        error("Cannot have more than 255 arguments.");

      ++argCount;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expect `)` after arguments.");
  return argCount;
}

static void and_(bool canAssign) {
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(PREC_AND);

  patchJump(endJump);
}

static void binary(bool canAssign) {
  TokenType operatorType = parser.previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
  case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
  case TOKEN_GREATER:       emitByte(OP_GREATER); break;
  case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
  case TOKEN_LESS:          emitByte(OP_LESS); break;
  case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
  case TOKEN_PLUS:          emitByte(OP_ADD); break;
  case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
  case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
  case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
  default:                  break;
  }
}

static void call(bool canAssign) {
  u8_t argCount = argumentList();
  emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign) {
  consume(TOKEN_IDENTIFIER, "Expect property name after `.`.");
  u8_t name = identifierConstant(&parser.previous);

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(OP_SET_PROPERTY, name);
  }
  else if (match(TOKEN_LEFT_PAREN)) {
    u8_t argCount = argumentList();
    emitBytes(OP_INVOKE, name);
    emitByte(argCount);
  }
  else {
    emitBytes(OP_GET_PROPERTY, name);
  }
}

static void literal(bool canAssign) {
  switch (parser.previous.type) {
  case KEYWORD_FALSE: emitByte(OP_FALSE); break;
  case KEYWORD_NIL:   emitByte(OP_NIL); break;
  case KEYWORD_TRUE:  emitByte(OP_TRUE); break;
  default:            return;
  }
}

static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect `)` after expression.");
}

static void number(bool canAssign) {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);
}

static void string(bool canAssign) {
  emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
  u8_t getOp, setOp;
  int arg = resolveLocal(current, &name);
  if (-1 != arg) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  }
  else if (-1 != (arg = resolveUpvalue(current, &name))) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  }
  else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (u8_t)arg);
  }
  else {
    emitBytes(getOp, (u8_t)arg);
  }
}

static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

static void super_(bool canAssign) {
  if (NULL == currentClass)
    error("Cannot use `super` outside of a class.");
  else if (!currentClass->hasSuperclass)
    error("Cannot use `super` in a class with no superclass.");

  consume(TOKEN_DOT, "Expect `.` after `super`.");
  consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
  u8_t name = identifierConstant(&parser.previous);

  namedVariable(syntheticToken("this"), false);

  if (match(TOKEN_LEFT_PAREN)) {
    u8_t argCount = argumentList();
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_SUPER_INVOKE, name);
    emitByte(argCount);
  }
  else {
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_GET_SUPER, name);
  }
}

static void this_(bool canAssign) {
  if (NULL == currentClass) {
    error("Cannot use `this` outside of a class.");
    return;
  }

  variable(false);
}

static void expression() {}
static void statement() {}
static void declaration() {}
static ParseRule* getRule(TokenType type) { return NULL; }
static void parsePrecedence(Precedence precedence) {}

ObjFunction* compile(const char* sourceCode) { return NULL; }
void markCompilerRoots() {}
