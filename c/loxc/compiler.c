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
}

ObjFunction* compile(const char* sourceCode) { return NULL; }
void markCompilerRoots() {}
