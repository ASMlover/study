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
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "vm.h"

#define WREN_MAX_LOCALS                     (256)
#define WREN_MAX_UPVALUES                   (256)
#define WREN_MAX_CONSTANTS                  (1 << 16)
#define WREN_MAX_JUMP                       (1 << 16)
#define WREN_MAX_INTERPOLATION_NESTING      (8)
#define WREN_ERROR_MESSAGE_SIZE             (80 + WREN_MAX_VARIABLE_NAME + 15)

typedef enum {
#undef WREN_TOKENDEF
#define WREN_TOKENDEF(k, s) k,
#include "token_type.h"
#undef WREN_TOKENDEF
} TokenType;

typedef struct {
	TokenType type;
	const char* start;
	int length;
	int lineno;
	Value value;
} Token;

typedef struct {
	WrenVM* vm;
	ObjModule* module;
	const char* sourceCode;
	const char* tokenStart;
	const char* currentChar;
	int currentLine;
	Token current;
	Token previous;

	int parens[WREN_MAX_INTERPOLATION_NESTING];
	int numParens;

	bool skipNewlines;
	bool printErrors;
	bool hasError;
} Parser;

typedef struct {
	const char* name;
	int length;
	int depth;
	bool isUpvalue;
} Local;

typedef struct {
	bool isLocal;
	int index;
} CompilerUpvalue;

typedef struct Loop {
	int start;
	int exitJump;
	int body;
	int scopeDepth;
	struct Loop* enclosing;
} Loop;

typedef enum {
	SIG_METHOD,
	SIG_GETTER,
	SIG_SETTER,
	SIG_SUBSCRIPT,
	SIG_SUBSCRIPT_SETTER,
	SIG_INITIALIZER,
} SignatureType;

typedef struct {
	const char* name;
	int length;
	SignatureType type;
	int arity;
} Signature;

typedef struct {
	ObjString* name;
	SymbolTable fields;
	IntBuffer methods;
	IntBuffer staticMethods;
	bool isForeign;
	bool inStatic;
	Signature* signature;
} ClassInfo;

struct Compiler {
	Parser* parser;
	struct Compiler* parent;

	Local locals[WREN_MAX_LOCALS];
	int numLocals;
	CompilerUpvalue upvalues[WREN_MAX_UPVALUES];

	int scopeDepth;
	int numSlots;
	Loop* loop;
	ClassInfo* enclosingClass;
	ObjFn* fn;
	ObjMap* constants;
};

typedef enum {
	SCOPE_LOCAL,
	SCOPE_UPVALUE,
	SCOPE_MODULE,
} Scope;

typedef struct {
	int index;
	Scope scope;
} Variable;

static const int stackEffects[] = {
#undef WREN_OPCODE
#define WREN_OPCODE(_, effect) effect,
#include "opcodes.h"
#undef WREN_OPCODE
};

static void printError(Parser* parser,
		int lineno, const char* label, const char* format, va_list args) {
	parser->hasError = true;
	if (!parser->printErrors)
		return;

	if (parser->vm->config.errorFn == NULL)
		return;

	char message[WREN_ERROR_MESSAGE_SIZE];
	int length = sprintf(message, "%s: ", label);
	length += vsprintf(message + length, format, args);
	ASSERT(length < WREN_ERROR_MESSAGE_SIZE, "Error should not exceed buffer.");

	parser->vm->config.errorFn(parser->vm,
			WREN_ERROR_COMPILE, parser->module->name->value, lineno, message);
}

static void lexError(Parser* parser, const char* format, ...) {
	va_list args;
	va_start(args, format);
	printError(parser, parser->currentLine, "ERROR", format, args);
	va_end(args);
}

ObjFn* wrenCompile(WrenVM* vm,
		ObjModule* module, const char* sourceCode, bool isExpression, bool printErrors) {
	return NULL;
}

void wrenBindMethodCode(ObjClass* classObj, ObjFn* fn) {}
void wrenMarkCompiler(WrenVM* vm, Compiler* compiler) {}
