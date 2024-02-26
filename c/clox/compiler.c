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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "scanner.h"

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,    // =
	PREC_OR,            // or
	PREC_AND,           // and
	PREC_EQUALITY,      // == !=
	PREC_COMPARISON,    // < > <= >=
	PREC_TERM,          // + -
	PREC_FACTOR,        // * /
	PREC_UNARY,         // ! -
	PREC_CALL,          // . ()
	PREC_PRIMARY,
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

typedef struct {
	Token name;
	int depth;
	bool isCaptured;
} Local;

typedef struct {
	uint8_t index;
	bool isLocal;
} Upvalue;

typedef enum {
	TYPE_FUNCTION,
	TYPE_INITIALIZER,
	TYPE_METHOD,
	TYPE_SCRIPT,
} FunctionType;

typedef struct Compiler {
	struct Compiler* enclosing;
	ObjFunction* function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int localCount;
	Upvalue upvalues[UINT8_COUNT];
	int scopeDepth;
} Compiler;

typedef struct ClassCompiler {
	struct ClassCompiler* enclosing;
	bool hasSuperclass;
} ClassCompiler;


Parser parser;
Compiler* current = NULL;
ClassCompiler* currentClass = NULL;

static Chunk* currentChunk() {
	return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
	if (parser.panicMode)
		return;
	parser.panicMode = true;

	fprintf(stderr, "[line %d] Error", token->lineno);
	if (token->type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	}
	else if (token->type == TOKEN_ERROR) {
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
		if (parser.current.type != TOKEN_ERROR)
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

static bool check(TokenType type) {
	return parser.current.type == type;
}

static bool match(TokenType type) {
	if (check(type)) {
		advance();
		return true;
	}
	return false;
}

static void emitByte(uint8_t byte) {
	writeChunk(currentChunk(), byte, parser.previous.lineno);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
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

static int emitJump(uint8_t instruction) {
	emitByte(instruction);
	emitByte(0xff);
	emitByte(0xff);
	return currentChunk()->count - 2;
}

static void emitReturn() {
	if (current->type == TYPE_INITIALIZER)
		emitBytes(OP_GET_LOCAL, 0);
	else
		emitByte(OP_NIL);

	emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
	int constant = addConstant(currentChunk(), value);
	if (constant > UINT8_MAX) {
		error("Too many constants in one chunk.");
		return 0;
	}

	return (uint8_t)constant;
}

static void emitConstant(Value value) {
	emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
	int jump = currentChunk()->count - offset - 2;

	if (jump > UINT16_MAX)
		error("Too much coed to jump over.");

	currentChunk()->code[offset] = (jump >> 8) & 0xff;
	currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
	compiler->enclosing = current;
	compiler->function = NULL;
	compiler->type = type;
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	compiler->function = newFunction();
	current = compiler;

	if (type != TYPE_SCRIPT)
		current->function->name = copyString(parser.previous.start, parser.previous.length);

	Local* local = &current->locals[current->localCount++];
	local->depth = 0;
	local->isCaptured = false;

	if (type != TYPE_FUNCTION) {
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

#if defined(CLOX_DEBUG_PRINT_CODE)
	if (!parser.hadError)
		disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
#endif

	current = current->enclosing;
	return function;
}

static void beginScope() {
	++current->scopeDepth;
}

static void endScope() {
	--current->scopeDepth;

	while (current->localCount > 0 &&
			current->locals[current->localCount - 1].depth > current->scopeDepth) {
		if (current->locals[current->localCount - 1].isCaptured)
			emitByte(OP_CLOSE_UPVALUE);
		else
			emitByte(OP_POP);

		--current->localCount;
	}
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token* name) {
	return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static int resolveLocal(Compiler* compiler, Token* name) {
	for (int i = compiler->localCount - 1; i >= 0; --i) {
		Local* local = &compiler->locals[i];
		if (identifiersEqual(name, &local->name)) {
			if (local->depth == -1)
				error("Cannot read local variable in its own initializer.");
			return i;
		}
	}
	return -1;
}

static int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal) {
	int upvalueCount = compiler->function->upvalueCount;

	for (int i = 0; i < upvalueCount; ++i) {
		Upvalue* upvalue = &compiler->upvalues[i];
		if (upvalue->index == index && upvalue->isLocal == isLocal)
			return i;
	}

	if (upvalueCount == UINT8_COUNT) {
		error("Too many closure variables in function.");
		return 0;
	}

	compiler->upvalues[upvalueCount].isLocal = isLocal;
	compiler->upvalues[upvalueCount].index = index;
	return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
	if (compiler->enclosing == NULL)
		return -1;

	int local = resolveLocal(compiler->enclosing, name);
	if (local != -1) {
		compiler->enclosing->locals[local].isCaptured = true;
		return addUpvalue(compiler, (uint8_t)local, true);
	}

	int upvalue = resolveUpvalue(compiler->enclosing, name);
	if (upvalue != -1)
		return addUpvalue(compiler, (uint8_t)upvalue, false);
	return -1;
}

static void addLocal(Token name) {
	if (current->localCount == UINT8_COUNT) {
		error("Too many local variables in function.");
		return;
	}

	Local* local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = -1;
	local->isCaptured = false;
}

static void declareVariable() {
	if (current->scopeDepth == 0)
		return;

	Token* name = &parser.previous;
	for (int i = current->localCount - 1; i >= 0; --i) {
		Local* local = &current->locals[i];
		if (local->depth != -1 && local->depth < current->scopeDepth)
			break;

		if (identifiersEqual(name, &local->name))
			error("Alreay a variable with this name in this scope.");
	}
	addLocal(*name);
}

ObjFunction* compile(const char* sourceCode) { return NULL; }
void markCompilerRoots() {}
