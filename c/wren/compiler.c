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
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "vm.h"
#include "debug.h"

#define WREN_MAX_LOCALS                     (256)
#define WREN_MAX_UPVALUES                   (256)
#define WREN_MAX_CONSTANTS                  (1 << 16)
#define WREN_MAX_JUMP                       (1 << 16)
#define WREN_MAX_INTERPOLATION_NESTING      (8)
#define WREN_ERROR_MESSAGE_SIZE             (80 + WREN_MAX_VARIABLE_NAME + 15)

#define RULE_UNUSED                         {NULL, NULL, NULL, PREC_NONE, NULL}
#define RULE_PREFIX(fn)                     {fn, NULL, NULL, PREC_NONE, NULL}
#define RULE_INFIX(prec, fn)                {NULL, fn, NULL, prec, NULL}
#define RULE_INFIX_OPERATOR(prec, name)     {NULL, infixOp, infixSignature, prec, name}
#define RULE_PREFIX_OPERATOR(name)          {unaryOp, NULL, unarySignature, PREC_NONE, name}
#define RULE_OPERATOR(name)                 {unaryOp, infixOp, mixedSignature, PREC_TERM, NULL}

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

typedef struct {
	const char* identifier;
	sz_t length;
	TokenType tokenType;
} Keyword;

typedef enum {
	PREC_NONE,
	PREC_LOWEST,
	PREC_ASSIGNMENT,    //
	PREC_CONDITIONAL,   // ?:
	PREC_LOGICAL_OR,    // ||
	PREC_LOGICAL_AND,   // &&
	PREC_EQUALITY,      // == !=
	PREC_IS,            // is
	PREC_COMPARISON,    // < > <= >=
	PREC_BITWISE_OR,    // |
	PREC_BITWISE_XOR,   // ^
	PREC_BITWISE_AND,   // &
	PREC_BITWISE_SHIFT, // << >>
	PREC_RANGE,         // .. ...
	PREC_TERM,          // + -
	PREC_FACTOR,        // * / %
	PREC_UNARY,         // unary - ! ~
	PREC_CALL,          // . () []
	PREC_PRIMARY,
} Precedence;

typedef void (*GrammarFn)(Compiler* compiler, bool canAssign);
typedef void (*SignatureFn)(Compiler* compiler, Signature* signature);

typedef struct {
	GrammarFn prefix;
	GrammarFn infix;
	SignatureFn method;
	Precedence precedence;
	const char* name;
} GrammarRule;

static const int stackEffects[] = {
#undef WREN_OPCODE
#define WREN_OPCODE(_, effect) effect,
#include "opcodes.h"
#undef WREN_OPCODE
};

static const Keyword keywords[] = {
	{"break",     5, KEYWORD_BREAK},
	{"class",     5, KEYWORD_CLASS},
	{"construct", 9, KEYWORD_CONSTRUCT},
	{"else",      4, KEYWORD_ELSE},
	{"false",     5, KEYWORD_FALSE},
	{"for",       3, KEYWORD_FOR},
	{"foreign",   7, KEYWORD_FOREIGN},
	{"if",        2, KEYWORD_IF},
	{"import",    6, KEYWORD_IMPORT},
	{"in",        2, KEYWORD_IN},
	{"is",        2, KEYWORD_IS},
	{"null",      4, KEYWORD_NULL},
	{"return",    6, KEYWORD_RETURN},
	{"static",    6, KEYWORD_STATIC},
	{"super",     5, KEYWORD_SUPER},
	{"this",      4, KEYWORD_THIS},
	{"true",      4, KEYWORD_TRUE},
	{"var",       3, KEYWORD_VAR},
	{"while",     5, KEYWORD_WHILE},
	{NULL,        0, TOKEN_EOF},
};

static GrammarRule* getRule(TokenType type);
static void expression(Compiler* compiler);
static void statement(Compiler* compiler);
static void definition(Compiler* compiler);
static void parsePrecedence(Compiler* compiler, Precedence precedence);

static void printError(Parser* parser,
		int lineno, const char* label, const char* format, va_list args) {
	parser->hasError = true;
	if (!parser->printErrors)
		return;

	if (NULL == parser->vm->config.errorFn)
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

static void error(Compiler* compiler, const char* format, ...) {
	Token* token = &compiler->parser->previous;
	if (token->type == TOKEN_ERROR)
		return;

	va_list args;
	va_start(args, format);
	if (token->type == TOKEN_LINE) {
		printError(compiler->parser, token->lineno, "Error at newline", format, args);
	}
	else if (token->type == TOKEN_EOF) {
		printError(compiler->parser, token->lineno, "Error at end of file", format, args);
	}
	else {
		char label[10 + WREN_MAX_VARIABLE_NAME + 4 + 1];
		if (token->length <= WREN_MAX_VARIABLE_NAME)
			sprintf(label, "Error at `%.*s`", token->length, token->start);
		else
			sprintf(label, "Error at `%.*s...`", WREN_MAX_VARIABLE_NAME, token->start);
		printError(compiler->parser, token->lineno, label, format, args);
	}
	va_end(args);
}

static int addConstant(Compiler* compiler, Value constant) {
	if (compiler->parser->hasError)
		return -1;

	if (NULL != compiler->constants) {
		Value existing = wrenMapGet(compiler->constants, constant);
		if (IS_NUM(existing))
			return (int)AS_NUM(existing);
	}

	if (compiler->fn->constants.count < WREN_MAX_CONSTANTS) {
		if (IS_OBJ(constant))
			wrenPushRoot(compiler->parser->vm, AS_OBJ(constant));
		wrenValueBufferWrite(compiler->parser->vm, &compiler->fn->constants, constant);
		if (IS_OBJ(constant))
			wrenPopRoot(compiler->parser->vm);

		if (NULL == compiler->constants)
			compiler->constants = wrenNewMap(compiler->parser->vm);
		wrenMapSet(compiler->parser->vm,
				compiler->constants, constant, NUM_VAL(compiler->fn->constants.count - 1));
	}
	else {
		error(compiler, "A function may only contain %d unique constants.", WREN_MAX_CONSTANTS);
	}

	return compiler->fn->constants.count - 1;
}

static void initCompiler(Compiler* compiler, Parser* parser, Compiler* parent, bool isMethod) {
	compiler->parser = parser;
	compiler->parent = parent;
	compiler->loop = NULL;
	compiler->enclosingClass = NULL;
	compiler->fn = NULL;
	compiler->constants = NULL;

	parser->vm->compiler = compiler;

	compiler->numLocals = 1;
	compiler->numSlots = compiler->numLocals;

	if (isMethod) {
		compiler->locals[0].name = "this";
		compiler->locals[0].length = 4;
	}
	else {
		compiler->locals[0].name = NULL;
		compiler->locals[0].length = 0;
	}

	compiler->locals[0].depth = -1;
	compiler->locals[0].isUpvalue = false;

	if (NULL == parent)
		compiler->scopeDepth = -1;
	else
		compiler->scopeDepth = 0;

	compiler->fn = wrenNewFunction(parser->vm, parser->module, compiler->numLocals);
}

static inline bool isAlpha(char c) {
	return isalpha(c) || c == '_';
}

static inline bool isAlnum(char c) {
	return isalnum(c) || c == '_';
}

static inline bool isDigit(char c) {
	return isdigit(c);
}

static bool isName(char c) {
	return isAlpha(c);
}

static char peekChar(Parser* parser) {
	return *parser->currentChar;
}

static char peekNextChar(Parser* parser) {
	if (peekChar(parser) == 0)
		return 0;
	return *(parser->currentChar + 1);
}

static char nextChar(Parser* parser) {
	char c = peekChar(parser);
	++parser->currentChar;
	if (c == '\n')
		++parser->currentLine;
	return c;
}

static bool matchChar(Parser* parser, char c) {
	if (peekChar(parser) != c)
		return false;

	nextChar(parser);
	return true;
}

static void makeToken(Parser* parser, TokenType type) {
	parser->current.type = type;
	parser->current.start = parser->tokenStart;
	parser->current.length = (int)(parser->currentChar - parser->tokenStart);
	parser->current.lineno = parser->currentLine;

	if (type == TOKEN_LINE)
		--parser->current.lineno;
}

static void twoCharToken(Parser* parser, char c, TokenType two, TokenType one) {
	makeToken(parser, matchChar(parser, c) ? two : one);
}

static void skipLineComment(Parser* parser) {
	while (peekChar(parser) != '\n' && peekChar(parser) != 0)
		nextChar(parser);
}

static void skipBlockComment(Parser* parser) {
	int nesting = 1;
	while (nesting > 0) {
		if (peekChar(parser) == 0) {
			lexError(parser, "Unterminated block comment.");
			return;
		}

		if (peekChar(parser) == '/' && peekNextChar(parser) == '*') {
			nextChar(parser);
			nextChar(parser);
			++nesting;
			continue;
		}
		if (peekChar(parser) == '*' && peekNextChar(parser) == '/') {
			nextChar(parser);
			nextChar(parser);
			--nesting;
			continue;
		}

		nextChar(parser);
	}
}

static int readHexDigit(Parser* parser) {
	char c = nextChar(parser);
	if (c >= '0' && c <= '0')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	--parser->currentChar;
	return -1;
}

static void makeNumber(Parser* parser, bool isHex) {
	errno = 0;
	if (isHex)
		parser->current.value = NUM_VAL(strtoll(parser->tokenStart, NULL, 16));
	else
		parser->current.value = NUM_VAL(strtod(parser->tokenStart, NULL));

	if (errno == ERANGE) {
		lexError(parser, "Number literal was too large (%d).", sizeof(long int));
		parser->current.value = NUM_VAL(0);
	}

	makeToken(parser, TOKEN_NUMBER);
}

static void readHexNumber(Parser* parser) {
	nextChar(parser);
	while (readHexDigit(parser) != -1)
		continue;

	makeNumber(parser, true);
}

static void readNumber(Parser* parser) {
	while (isDigit(peekChar(parser)))
		nextChar(parser);

	if (peekChar(parser) == '.' && isDigit(peekNextChar(parser))) {
		nextChar(parser);
		while (isDigit(peekChar(parser)))
			nextChar(parser);
	}

	if (matchChar(parser, 'e') || matchChar(parser, 'E')) {
		matchChar(parser, '-');
		if (!isDigit(peekChar(parser)))
			lexError(parser, "Unterminated scientific notation.");

		while (isDigit(peekChar(parser)))
			nextChar(parser);
	}

	makeNumber(parser, false);
}

static void readName(Parser* parser, TokenType type) {
	while (isName(peekChar(parser)) || isDigit(peekChar(parser)))
		nextChar(parser);

	sz_t length = parser->currentChar - parser->tokenStart;
	for (int i = 0; NULL != keywords[i].identifier; ++i) {
		if (length == keywords[i].length &&
				memcmp(parser->tokenStart, keywords[i].identifier, length) == 0) {
			type = keywords[i].tokenType;
			break;
		}
	}

	makeToken(parser, type);
}

static int readHexEscape(Parser* parser, int digits, const char* description) {
	int value = 0;
	for (int i = 0; i < digits; ++i) {
		if (peekChar(parser) == '"' || peekChar(parser) == 0) {
			lexError(parser, "Incomplete %s escape sequence.", description);
			++parser->currentChar;
			break;
		}

		int digit = readHexDigit(parser);
		if (digit == -1) {
			lexError(parser, "Invalid %s escape sequence.", description);
			break;
		}

		value = (value * 16) | digit;
	}
	return value;
}

static void readUnicodeEscape(Parser* parser, ByteBuffer* string, int length) {
	int value = readHexEscape(parser, length, "Unicode");
	int numBytes = wrenUtf8EncodeNumBytes(value);
	if (numBytes != 0) {
		wrenByteBufferFill(parser->vm, string, 0, numBytes);
		wrenUtf8Encode(value, string->data + string->count - numBytes);
	}
}

static void readString(Parser* parser) {
	ByteBuffer string;
	TokenType type = TOKEN_STRING;
	wrenByteBufferInit(&string);

	for (;;) {
		char c = nextChar(parser);
		if (c == '"')
			break;

		if (c == 0) {
			lexError(parser, "Unterminated string.");
			--parser->currentChar;
			break;
		}

		if (c == '%') {
			if (parser->numParens < WREN_MAX_INTERPOLATION_NESTING) {
				if (nextChar(parser) != '(')
					lexError(parser, "Expect `(` after `%%`.");

				parser->parens[parser->numParens++] = 1;
				type = TOKEN_INTERPOLATION;
				break;
			}

			lexError(parser, "Interpolation may only nest %d levels deep.", WREN_MAX_INTERPOLATION_NESTING);
		}
		if (c == '\\') {
			switch (nextChar(parser)) {
			case '"':  wrenByteBufferWrite(parser->vm, &string, '"'); break;
			case '\\': wrenByteBufferWrite(parser->vm, &string, '\\'); break;
			case '%':  wrenByteBufferWrite(parser->vm, &string, '%'); break;
			case '0':  wrenByteBufferWrite(parser->vm, &string, '\0'); break;
			case 'a':  wrenByteBufferWrite(parser->vm, &string, '\a'); break;
			case 'b':  wrenByteBufferWrite(parser->vm, &string, '\b'); break;
			case 'f':  wrenByteBufferWrite(parser->vm, &string, '\f'); break;
			case 'n':  wrenByteBufferWrite(parser->vm, &string, '\n'); break;
			case 'r':  wrenByteBufferWrite(parser->vm, &string, '\r'); break;
			case 't':  wrenByteBufferWrite(parser->vm, &string, '\t'); break;
			case 'u':  readUnicodeEscape(parser, &string, 4); break;
			case 'U':  readUnicodeEscape(parser, &string, 8); break;
			case 'v':  wrenByteBufferWrite(parser->vm, &string, '\v'); break;
			case 'x':  wrenByteBufferWrite(parser->vm, &string, (u8_t)readHexEscape(parser, 2, "bytes")); break;
			default:   lexError(parser, "Invalid escape character `%c`.", *(parser->currentChar - 1)); break;
			}
		}
		else {
			wrenByteBufferWrite(parser->vm, &string, c);
		}
	}

	parser->current.value = wrenNewStringLength(parser->vm, (char*)string.data, string.count);
	wrenByteBufferClear(parser->vm, &string);
	makeToken(parser, type);
}

static void nextToken(Parser* parser) {
	parser->previous = parser->current;
	if (parser->current.type == TOKEN_EOF)
		return;

	while (peekChar(parser) != 0) {
		parser->tokenStart = parser->currentChar;

		char c = nextChar(parser);
		switch (c) {
		case '(':
			if (parser->numParens > 0)
				++parser->parens[parser->numParens - 1];
			makeToken(parser, TOKEN_LEFT_PAREN);
			return;
		case ')':
			if (parser->numParens > 0 && --parser->parens[parser->numParens - 1] == 0) {
				--parser->numParens;
				readString(parser);
				return;
			}
			makeToken(parser, TOKEN_RIGHT_PAREN);
			return;
		case '[': makeToken(parser, TOKEN_LEFT_BRACKET); return;
		case ']': makeToken(parser, TOKEN_RIGHT_BRACKET); return;
		case '{': makeToken(parser, TOKEN_LEFT_BRACE); return;
		case '}': makeToken(parser, TOKEN_RIGHT_BRACE); return;
		case ':': makeToken(parser, TOKEN_COLON); return;
		case ',': makeToken(parser, TOKEN_COMMA); return;
		case '*': makeToken(parser, TOKEN_STAR); return;
		case '%': makeToken(parser, TOKEN_PERCENT); return;
		case '^': makeToken(parser, TOKEN_CARET); return;
		case '+': makeToken(parser, TOKEN_PLUS); return;
		case '-': makeToken(parser, TOKEN_MINUS); return;
		case '~': makeToken(parser, TOKEN_TILDE); return;
		case '?': makeToken(parser, TOKEN_QUESTION); return;
		case '|': twoCharToken(parser, '|', TOKEN_PIPEPIPE, TOKEN_PIPE); return;
		case '&': twoCharToken(parser, '&', TOKEN_AMPAMP, TOKEN_AMP); return;
		case '=': twoCharToken(parser, '=', TOKEN_EQEQ, TOKEN_EQ); return;
		case '!': twoCharToken(parser, '=', TOKEN_BANGEQ, TOKEN_BANG); return;
		case '.':
			if (matchChar(parser, '.')) {
				twoCharToken(parser, '.', TOKEN_DOTDOTDOT, TOKEN_DOTDOT);
				return;
			}
			makeToken(parser, TOKEN_DOT);
			return;
		case '/':
			if (matchChar(parser, '/')) {
				skipLineComment(parser);
				break;
			}
			if (matchChar(parser, '*')) {
				skipBlockComment(parser);
				break;
			}
			makeToken(parser, TOKEN_SLASH);
			return;
		case '<':
			if (matchChar(parser, '<'))
				makeToken(parser, TOKEN_LTLT);
			else
				twoCharToken(parser, '=', TOKEN_LTEQ, TOKEN_LT);
			return;
		case '>':
			if (matchChar(parser, '>'))
				makeToken(parser, TOKEN_GTGT);
			else
				twoCharToken(parser, '=', TOKEN_GTEQ, TOKEN_GT);
			return;
		case '\n': makeToken(parser, TOKEN_LINE); return;
		case ' ':
		case '\r':
		case '\t':
			while (peekChar(parser) == ' ' || peekChar(parser) == '\r' || peekChar(parser) == '\t')
				nextChar(parser);
			break;
		case '"': readString(parser); return;
		case '_': readName(parser, peekChar(parser) == '_' ? TOKEN_STATIC_FIELD : TOKEN_FIELD); return;
		case '0':
			if (peekChar(parser) == 'x') {
				readHexNumber(parser);
				return;
			}
			readNumber(parser);
			return;
		default:
			if (parser->currentLine == 1 && c == '#' && peekChar(parser) == '!') {
				skipLineComment(parser);
				break;
			}
			if (isName(c)) {
				readName(parser, TOKEN_NAME);
			}
			else if (isDigit(c)) {
				readNumber(parser);
			}
			else {
				if (c >= 32 && c <= 126)
					lexError(parser, "Invalid character `%c`.", c);
				else
					lexError(parser, "Invalid byte 0x%x.", (u8_t)c);
				parser->current.type = TOKEN_ERROR;
				parser->current.length = 0;
			}
			return;
		}
	}

	parser->tokenStart = parser->currentChar;
	makeToken(parser, TOKEN_EOF);
}

static inline TokenType peek(Compiler* compiler) {
	return compiler->parser->current.type;
}

static bool match(Compiler* compiler, TokenType expected) {
	if (peek(compiler) != expected)
		return false;

	nextToken(compiler->parser);
	return true;
}

static void consume(Compiler* compiler, TokenType expected, const char* errorMessage) {
	nextToken(compiler->parser);
	if (compiler->parser->previous.type != expected) {
		error(compiler, errorMessage);
		if (compiler->parser->current.type == expected)
			nextToken(compiler->parser);
	}
}

static bool matchLine(Compiler* compiler) {
	if (!match(compiler, TOKEN_LINE))
		return false;

	while (match(compiler, TOKEN_LINE)) {
	}
	return true;
}

static void ignoreNewlines(Compiler* compiler) {
	matchLine(compiler);
}

static void consumeLine(Compiler* compiler, const char* errorMessage) {
	consume(compiler, TOKEN_LINE, errorMessage);
	ignoreNewlines(compiler);
}

static int emitByte(Compiler* compiler, int byte) {
	wrenByteBufferWrite(compiler->parser->vm, &compiler->fn->code, (u8_t)byte);
	wrenIntBufferWrite(compiler->parser->vm,
			&compiler->fn->debug->sourceLines, compiler->parser->previous.lineno);
	return compiler->fn->code.count - 1;
}

static void emitOp(Compiler* compiler, Code instruction) {
	emitByte(compiler, instruction);

	compiler->numSlots += stackEffects[instruction];
	if (compiler->numSlots > compiler->fn->maxSlots)
		compiler->fn->maxSlots = compiler->numSlots;
}

static void emitShort(Compiler* compiler, int arg) {
	emitByte(compiler, (arg >> 8) & 0xff);
	emitByte(compiler, arg & 0xff);
}

static int emitByteArg(Compiler* compiler, Code instruction, int arg) {
	emitOp(compiler, instruction);
	return emitByte(compiler, arg);
}

static void emitShortArg(Compiler* compiler, Code instruction, int arg) {
	emitOp(compiler, instruction);
	emitShort(compiler, arg);
}

static int emitJump(Compiler* compiler, Code instruction) {
	emitOp(compiler, instruction);
	emitByte(compiler, 0xff);
	return emitByte(compiler, 0xff) - 1;
}

static void emitConstant(Compiler* compiler, Value value) {
	int constant = addConstant(compiler, value);
	emitShortArg(compiler, CODE_CONSTANT, constant);
}

static int addLocal(Compiler* compiler, const char* name, int length) {
	Local* local = &compiler->locals[compiler->numLocals];
	local->name = name;
	local->length = length;
	local->depth = compiler->scopeDepth;
	local->isUpvalue = false;
	return compiler->numLocals++;
}

static int declareVariable(Compiler* compiler, Token* token) {
	if (NULL == token)
		token = &compiler->parser->previous;

	if (token->length > WREN_MAX_VARIABLE_NAME)
		error(compiler, "Variable name cannot be longer than %d characters.", WREN_MAX_VARIABLE_NAME);

	if (compiler->scopeDepth == -1) {
		int symbol = wrenDefineVariable(compiler->parser->vm,
				compiler->parser->module, token->start, token->length, NULL_VAL);
		if (symbol == -1)
			error(compiler, "Module variable is already defined.");
		else if (symbol == -2)
			error(compiler, "Too many module variables defined.");
		return symbol;
	}

	for (int i = compiler->numLocals - 1; i >= 0; --i) {
		Local* local = &compiler->locals[i];
		if (local->depth < compiler->scopeDepth)
			break;

		if (local->length == token->length && memcmp(local->name, token->start, token->length) == 0) {
			error(compiler, "Variable is already declared in this scope.");
			return i;
		}
	}

	if (compiler->numLocals == WREN_MAX_LOCALS) {
		error(compiler, "Cannot declare more than %d variables in one scope.", WREN_MAX_LOCALS);
		return -1;
	}

	return addLocal(compiler, token->start, token->length);
}

static int declareNamedVariable(Compiler* compiler) {
	consume(compiler, TOKEN_NAME, "Expect variable name.");
	return declareVariable(compiler, NULL);
}

static void defineVariable(Compiler* compiler, int symbol) {
	if (compiler->scopeDepth >= 0)
		return;

	emitShortArg(compiler, CODE_STORE_MODULE_VAR, symbol);
	emitOp(compiler, CODE_POP);
}

static void pushScope(Compiler* compiler) {
	++compiler->scopeDepth;
}

static int discardLocals(Compiler* compiler, int depth) {
	ASSERT(compiler->scopeDepth > -1, "Cannot exit top-level scope.");

	int local = compiler->numLocals - 1;
	while (local >= 0 && compiler->locals[local].depth >= depth) {
		if (compiler->locals[local].isUpvalue)
			emitByte(compiler, CODE_CLOSE_UPVALUE);
		else
			emitByte(compiler, CODE_POP);

		--local;
	}
	return compiler->numLocals - local - 1;
}

static void popScope(Compiler* compiler) {
	int popped = discardLocals(compiler, compiler->scopeDepth);
	compiler->numLocals -= popped;
	compiler->numSlots -= popped;
	--compiler->scopeDepth;
}

static int resolveLocal(Compiler* compiler, const char* name, int length) {
	for (int i = compiler->numLocals - 1; i >= 0; --i) {
		if (compiler->locals[i].length == length &&
				memcmp(name, compiler->locals[i].name, length) == 0)
			return i;
	}
	return -1;
}

static int addUpvalue(Compiler* compiler, bool isLocal, int index) {
	for (int i = 0; i < compiler->fn->numUpvalues; ++i) {
		CompilerUpvalue* upvalue = &compiler->upvalues[i];
		if (upvalue->index == index && upvalue->isLocal == isLocal)
			return i;
	}

	compiler->upvalues[compiler->fn->numUpvalues].isLocal = isLocal;
	compiler->upvalues[compiler->fn->numUpvalues].index = index;
	return compiler->fn->numUpvalues++;
}

static int findUpvalue(Compiler* compiler, const char* name, int length) {
	if (NULL == compiler->parent)
		return -1;

	if (name[0] != '_' && NULL != compiler->parent->enclosingClass)
		return -1;

	int local = resolveLocal(compiler->parent, name, length);
	if (local != -1) {
		compiler->parent->locals[local].isUpvalue = true;
		return addUpvalue(compiler, true, local);
	}

	int upvalue = findUpvalue(compiler->parent, name, length);
	if (upvalue != -1)
		return addUpvalue(compiler, false, upvalue);

	return -1;
}

static Variable resolveNonmodule(Compiler* compiler, const char* name, int length) {
	Variable variable;
	variable.scope = SCOPE_LOCAL;
	variable.index = resolveLocal(compiler, name, length);
	if (variable.index != -1)
		return variable;

	variable.scope = SCOPE_UPVALUE;
	variable.index = findUpvalue(compiler, name, length);
	return variable;
}

static Variable resolveName(Compiler* compiler, const char* name, int length) {
	Variable variable = resolveNonmodule(compiler, name, length);
	if (variable.index != -1)
		return variable;

	variable.scope = SCOPE_MODULE;
	variable.index = wrenSymbolTableFind(&compiler->parser->module->variableNames, name, length);
	return variable;
}

static void loadLocal(Compiler* compiler, int slot) {
	if (slot <= 8) {
		emitOp(compiler, (Code)(CODE_LOAD_LOCAL_0 + slot));
		return;
	}
	emitByteArg(compiler, CODE_LOAD_LOCAL, slot);
}

static ObjFn* endCompiler(Compiler* compiler, const char* debugName, int debugNameLength) {
	if (compiler->parser->hasError) {
		compiler->parser->vm->compiler = compiler->parent;
		return NULL;
	}

	emitOp(compiler, CODE_END);
	wrenFunctionBindName(compiler->parser->vm, compiler->fn, debugName, debugNameLength);

	if (NULL != compiler->parent) {
		int constant = addConstant(compiler, OBJ_VAL(compiler->fn));
		emitShortArg(compiler->parent, CODE_CLOSURE, constant);
		for (int i = 0; i < compiler->fn->numUpvalues; ++i) {
			emitByte(compiler->parent, compiler->upvalues[i].isLocal ? 1 : 0);
			emitByte(compiler->parent, compiler->upvalues[i].index);
		}
	}

	compiler->parser->vm->compiler = compiler->parent;

#if WREN_DEBUG_DUMP_COMPILED_CODE
	wrenDumpCode(compiler->parser->vm, compiler->fn);
#endif

	return compiler->fn;
}

static void patchJump(Compiler* compiler, int offset) {
	int jump = compiler->fn->code.count - offset - 2;
	if (jump > WREN_MAX_JUMP)
		error(compiler, "Too mush code to jump over.");

	compiler->fn->code.data[offset] = (jump >> 8) & 0xff;
	compiler->fn->code.data[offset + 1] = jump & 0xff;
}

static bool finishBlock(Compiler* compiler) {
	if (match(compiler, TOKEN_RIGHT_BRACE))
		return false;

	if (!matchLine(compiler)) {
		expression(compiler);
		consume(compiler, TOKEN_RIGHT_BRACE, "Expect `}` at end of block.");
		return true;
	}

	if (match(compiler, TOKEN_RIGHT_BRACE))
		return false;

	do {
		definition(compiler);
		consumeLine(compiler, "Expect newline after statement");
	} while (peek(compiler) != TOKEN_RIGHT_BRACE && peek(compiler) != TOKEN_EOF);

	consume(compiler, TOKEN_RIGHT_BRACE, "expect `}` at end of block.");
	return false;
}

static void finishBody(Compiler* compiler, bool isInitializer) {
	bool isExpressionBody = finishBlock(compiler);
	if (isInitializer) {
		if (isExpressionBody)
			emitOp(compiler, CODE_POP);
		emitOp(compiler, CODE_LOAD_LOCAL_0);
	}
	else if (!isExpressionBody) {
		emitOp(compiler, CODE_NULL);
	}

	emitOp(compiler, CODE_RETURN);
}

static void validateNumParameters(Compiler* compiler, int numArgs) {
	if (numArgs == WREN_MAX_PARAMETERS + 1)
		error(compiler, "Method cannot have more than %d parameters.", WREN_MAX_PARAMETERS);
}

static void finishParameterList(Compiler* compiler, Signature* signature) {
	do {
		ignoreNewlines(compiler);
		validateNumParameters(compiler, ++signature->arity);
		declareNamedVariable(compiler);
	} while (match(compiler, TOKEN_COMMA));
}

static int methodSymbol(Compiler* compiler, const char* name, int length) {
	return wrenSymbolTableEnsure(compiler->parser->vm,
			&compiler->parser->vm->methodNames, name, length);
}

static void signatureParameterList(char name[WREN_MAX_METHOD_SIGNATURE],
		int* length, int numParens, char leftBracket, char rightBracket) {
	name[(*length)++] = leftBracket;
	for (int i = 0; i < numParens && i < WREN_MAX_PARAMETERS; ++i) {
		if (i > 0)
			name[(*length)++] = ',';
		name[(*length)++] = '_';
	}
	name[(*length)++] = rightBracket;
}

static void signatureToString(Signature* signature,
		char name[WREN_MAX_METHOD_SIGNATURE], int* length) {
	*length = 0;

	memcpy(name + *length, signature->name, signature->length);
	*length += signature->length;

	switch (signature->type) {
	case SIG_METHOD: signatureParameterList(name, length, signature->arity, '(', ')'); break;
	case SIG_GETTER: break;
	case SIG_SETTER:
		name[(*length)++] = '=';
		signatureParameterList(name, length, 1, '(', ')');
		break;
	case SIG_SUBSCRIPT:
		signatureParameterList(name, length, signature->arity, '[', ']');
		break;
	case SIG_SUBSCRIPT_SETTER:
		signatureParameterList(name, length, signature->arity - 1, '[', ']');
		name[(*length)++] = '=';
		signatureParameterList(name, length, 1, '(', ')');
		break;
	case SIG_INITIALIZER:
		memcpy(name, "init", 5);
		memcpy(name + 5, signature->name, signature->length);
		*length = 5 + signature->length;
		signatureParameterList(name, length, signature->arity, '(', ')');
		break;
	}
	name[*length] = 0;
}

static int signatureSymbol(Compiler* compiler, Signature* signature) {
	char name[WREN_MAX_METHOD_SIGNATURE];
	int length;
	signatureToString(signature, name, &length);

	return methodSymbol(compiler, name, length);
}

static Signature signatureFromToken(Compiler* compiler, SignatureType type) {
	Signature signature;

	Token* token = &compiler->parser->previous;
	signature.name = token->start;
	signature.length = token->length;
	signature.type = type;
	signature.arity = 0;

	if (signature.length > WREN_MAX_METHOD_NAME) {
		error(compiler, "Method names cannot be longer than %d characters.", WREN_MAX_METHOD_NAME);
		signature.length = WREN_MAX_METHOD_NAME;
	}
	return signature;
}

static void finishArgumentList(Compiler* compiler, Signature* signature) {
	do {
		ignoreNewlines(compiler);
		validateNumParameters(compiler, ++signature->arity);
		expression(compiler);
	} while (match(compiler, TOKEN_COMMA));

	ignoreNewlines(compiler);
}

static void callSignature(Compiler* compiler, Code instruction, Signature* signature) {
	int symbol = signatureSymbol(compiler, signature);
	emitShortArg(compiler, (Code)(instruction + signature->arity), symbol);

	if (instruction == CODE_SUPER_0)
		emitShort(compiler, addConstant(compiler, NULL_VAL));
}

static void callMethod(Compiler* compiler, int numArgs, const char* name, int length) {
	int symbol = methodSymbol(compiler, name, length);
	emitShortArg(compiler, (Code)(CODE_CALL_0 + numArgs), symbol);
}

static void methodCall(Compiler* compiler, Code instruction, Signature* signature) {
	Signature called = {signature->name, signature->length, SIG_GETTER, 0};

	if (match(compiler, TOKEN_LEFT_PAREN)) {
		called.type = SIG_METHOD;
		if (peek(compiler) != TOKEN_RIGHT_PAREN)
			finishArgumentList(compiler, &called);
		consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after arguments.");
	}

	if (match(compiler, TOKEN_LEFT_BRACE)) {
		called.type = SIG_METHOD;
		++called.arity;

		Compiler fnCompiler;
		initCompiler(&fnCompiler, compiler->parser, compiler, false);

		Signature fnSignature = {"", 0, SIG_METHOD, 0};
		if (match(compiler, TOKEN_PIPE)) {
			finishParameterList(&fnCompiler, &fnSignature);
			consume(compiler, TOKEN_PIPE, "Expect `|` after function parameters.");
		}

		fnCompiler.fn->arity = fnSignature.arity;
		finishBody(&fnCompiler, false);

		char blockName[WREN_MAX_METHOD_SIGNATURE + 15];
		int blockLength;
		signatureToString(&called, blockName, &blockLength);
		memmove(blockName + blockLength, "block argument", 16);

		endCompiler(&fnCompiler, blockName, blockLength + 15);
	}

	if (signature->type == SIG_INITIALIZER) {
		if (called.type != SIG_METHOD)
			error(compiler, "A superclass constructor must have an argument list.");
		called.type = SIG_INITIALIZER;
	}

	callSignature(compiler, instruction, &called);
}

static void namedCall(Compiler* compiler, bool canAssign, Code instruction) {
	Signature signature = signatureFromToken(compiler, SIG_GETTER);

	if (canAssign && match(compiler, TOKEN_EQ)) {
		ignoreNewlines(compiler);

		signature.type = SIG_SETTER;
		signature.arity = 1;

		expression(compiler);
		callSignature(compiler, instruction, &signature);
	}
	else {
		methodCall(compiler, instruction, &signature);
	}
}

static void loadVariable(Compiler* compiler, Variable variable) {
	switch (variable.scope) {
	case SCOPE_LOCAL:   loadLocal(compiler, variable.index); break;
	case SCOPE_UPVALUE: emitByteArg(compiler, CODE_LOAD_UPVALUE, variable.index); break;
	case SCOPE_MODULE:  emitShortArg(compiler, CODE_LOAD_MODULE_VAR, variable.index); break;
	default: UNREACHABLE();
	}
}

static void loadThis(Compiler* compiler) {
	loadVariable(compiler, resolveNonmodule(compiler, "this", 4));
}

static void loadCoreVariable(Compiler* compiler, const char* name) {
	int symbol = wrenSymbolTableFind(&compiler->parser->module->variableNames, name, strlen(name));
	ASSERT(symbol != -1, "Should have already defined code name.");
	emitShortArg(compiler, CODE_LOAD_MODULE_VAR, symbol);
}

static void grouping(Compiler* compiler, bool canAssign) {
	expression(compiler);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after expression.");
}

static void list(Compiler* compiler, bool canAssign) {
	loadCoreVariable(compiler, "List");
	callMethod(compiler, 0, "new()", 5);

	do {
		ignoreNewlines(compiler);

		if (peek(compiler) == TOKEN_RIGHT_BRACKET)
			break;

		expression(compiler);
		callMethod(compiler, 1, "addCore_(_)", 11);
	} while (match(compiler, TOKEN_COMMA));

	ignoreNewlines(compiler);
	consume(compiler, TOKEN_RIGHT_BRACKET, "Expect `]` after list elements.");
}

static void map(Compiler* compiler, bool canAssign) {
	loadCoreVariable(compiler, "Map");
	callMethod(compiler, 0, "new()", 5);

	do {
		ignoreNewlines(compiler);

		if (peek(compiler) == TOKEN_RIGHT_BRACE)
			break;

		parsePrecedence(compiler, PREC_UNARY);
		consume(compiler, TOKEN_COLON, "Expect `:` after map key.");
		ignoreNewlines(compiler);

		expression(compiler);
		callMethod(compiler, 2, "addCore_(_,_)", 13);
	} while (match(compiler, TOKEN_COMMA));

	ignoreNewlines(compiler);
	consume(compiler, TOKEN_RIGHT_BRACE, "Expect `}` after map entries.");
}

static void unaryOp(Compiler* compiler, bool canAssign) {
	GrammarRule* rule = getRule(compiler->parser->previous.type);
	ignoreNewlines(compiler);
	parsePrecedence(compiler, (Precedence)(PREC_UNARY + 1));

	callMethod(compiler, 0, rule->name, 1);
}

static void boolean(Compiler* compiler, bool canAssign) {
	emitOp(compiler, compiler->parser->previous.type == KEYWORD_FALSE ? CODE_FALSE : CODE_TRUE);
}

static Compiler* getEnclosingClassCompiler(Compiler* compiler) {
	while (NULL != compiler) {
		if (NULL != compiler->enclosingClass)
			return compiler;

		compiler = compiler->parent;
	}
	return NULL;
}

static ClassInfo* getEnclosingClass(Compiler* compiler) {
	compiler = getEnclosingClassCompiler(compiler);
	return NULL == compiler ? NULL : compiler->enclosingClass;
}

static void field(Compiler* compiler, bool canAssign) {
	int field = 255;
	ClassInfo* enclosingClass = getEnclosingClass(compiler);

	if (NULL == enclosingClass) {
		error(compiler, "Cannot reference a field outside of a class definition.");
	}
	else if (enclosingClass->isForeign) {
		error(compiler, "Cannot define fields in a foreign class.");
	}
	else if (enclosingClass->inStatic) {
		error(compiler, "Cannot use an instance field in a static method.");
	}
	else {
		field = wrenSymbolTableEnsure(compiler->parser->vm,
				&enclosingClass->fields, compiler->parser->previous.start, compiler->parser->previous.length);
		if (field >= WREN_MAX_FIELDS)
			error(compiler, "A class can only have %d fields.", WREN_MAX_FIELDS);
	}

	bool isLoad = true;
	if (canAssign && match(compiler, TOKEN_EQ)) {
		expression(compiler);
		isLoad = false;
	}

	if (NULL != compiler->parent &&
			compiler->parent->enclosingClass == enclosingClass) {
		emitByteArg(compiler, isLoad ? CODE_LOAD_FIELD_THIS : CODE_STORE_FIELD_THIS, field);
	}
	else {
		loadThis(compiler);
		emitByteArg(compiler, isLoad ? CODE_LOAD_FIELD : CODE_STORE_FIELD, field);
	}
}

static void bareName(Compiler* compiler, bool canAssign, Variable variable) {
	if (canAssign && match(compiler, TOKEN_EQ)) {
		expression(compiler);

		switch (variable.scope) {
		case SCOPE_LOCAL:   emitByteArg(compiler, CODE_STORE_LOCAL, variable.index); break;
		case SCOPE_UPVALUE: emitByteArg(compiler, CODE_STORE_UPVALUE, variable.index); break;
		case SCOPE_MODULE:  emitShortArg(compiler, CODE_STORE_MODULE_VAR, variable.index); break;
		default:            UNREACHABLE();
		}
		return;
	}

	loadVariable(compiler, variable);
}

static void staticField(Compiler* compiler, bool canAssign) {
	Compiler* classCompiler = getEnclosingClassCompiler(compiler);
	if (NULL == classCompiler) {
		error(compiler, "Cannot use a static field outside of a class definition.");
		return;
	}

	Token* token = &compiler->parser->previous;
	if (resolveLocal(classCompiler, token->start, token->length) == -1) {
		int symbol = declareVariable(classCompiler, NULL);
		emitOp(classCompiler, CODE_NULL);
		defineVariable(classCompiler, symbol);
	}

	Variable variable = resolveName(compiler, token->start, token->length);
	bareName(compiler, canAssign, variable);
}

static inline bool isLocalName(const char* name) {
	return name[0] >= 'a' && name[0] <= 'z';
}

static void name(Compiler* compiler, bool canAssign) {
	Token* token = &compiler->parser->previous;
	Variable variable = resolveNonmodule(compiler, token->start, token->length);
	if (variable.index != -1) {
		bareName(compiler, canAssign, variable);
		return;
	}

	if (isLocalName(token->start) && NULL != getEnclosingClass(compiler)) {
		loadThis(compiler);
		namedCall(compiler, canAssign, CODE_CALL_0);
		return;
	}

	variable.scope = SCOPE_MODULE;
	variable.index = wrenSymbolTableFind(&compiler->parser->module->variableNames, token->start, token->length);
	if (variable.index == -1) {
		if (isLocalName(token->start)) {
			error(compiler, "Undefined variable.");
			return;
		}

		variable.index = wrenDeclareVariable(compiler->parser->vm,
				compiler->parser->module, token->start, token->length, token->lineno);
		if (variable.index == -2)
			error(compiler, "Too many module variables defined.");
	}

	bareName(compiler, canAssign, variable);
}

static void null(Compiler* compiler, bool canAssign) {
	emitOp(compiler, CODE_NULL);
}

static void literal(Compiler* compiler, bool canAssign) {
	emitConstant(compiler, compiler->parser->previous.value);
}

static void stringInterpolation(Compiler* compiler, bool canAssign) {
	loadCoreVariable(compiler, "List");
	callMethod(compiler, 0, "new()", 5);

	do {
		literal(compiler, false);
		callMethod(compiler, 1, "addCore_(_)", 11);

		ignoreNewlines(compiler);
		expression(compiler);
		callMethod(compiler, 1, "addCore_(_)", 11);

		ignoreNewlines(compiler);
	} while (match(compiler, TOKEN_INTERPOLATION));

	consume(compiler, TOKEN_STRING, "Expect end of string interpolation.");
	literal(compiler, false);
	callMethod(compiler, 1, "addCore_(_)", 11);

	callMethod(compiler, 0, "join()", 6);
}

static void super_(Compiler* compiler, bool canAssign) {
	ClassInfo* enclosingClass = getEnclosingClass(compiler);
	if (NULL == enclosingClass)
		error(compiler, "Cannot use `super` outside of a method");

	loadThis(compiler);
	if (match(compiler, TOKEN_DOT)) {
		consume(compiler, TOKEN_NAME, "Expect method name after `super`.");
		namedCall(compiler, canAssign, CODE_SUPER_0);
	}
	else if (NULL != enclosingClass) {
		methodCall(compiler, CODE_SUPER_0, enclosingClass->signature);
	}
}

static void this_(Compiler* compiler, bool canAssign) {
	if (NULL == getEnclosingClass(compiler)) {
		error(compiler, "Cannot use `this` outside of a method.");
		return;
	}
	loadThis(compiler);
}

static void subscript(Compiler* compiler, bool canAssign) {
	Signature signature = {"", 0, SIG_SUBSCRIPT, 0};

	finishArgumentList(compiler, &signature);
	consume(compiler, TOKEN_RIGHT_BRACKET, "Expect `]` after arguments.");

	if (canAssign && match(compiler, TOKEN_EQ)) {
		signature.type = SIG_SUBSCRIPT_SETTER;

		validateNumParameters(compiler, ++signature.arity);
		expression(compiler);
	}
	callSignature(compiler, CODE_CALL_0, &signature);
}

static void call(Compiler* compiler, bool canAssign) {
	ignoreNewlines(compiler);
	consume(compiler, TOKEN_NAME, "Expect method name after `.`.");
	namedCall(compiler, canAssign, CODE_CALL_0);
}

static void and_(Compiler* compiler, bool canAssign) {
	ignoreNewlines(compiler);

	int jump = emitJump(compiler, CODE_AND);
	parsePrecedence(compiler, PREC_LOGICAL_AND);
	patchJump(compiler, jump);
}

static void or_(Compiler* compiler, bool canAssign) {
	ignoreNewlines(compiler);

	int jump = emitJump(compiler, CODE_OR);
	parsePrecedence(compiler, PREC_LOGICAL_OR);
	patchJump(compiler, jump);
}

static void conditional(Compiler* compiler, bool canAssign) {
	ignoreNewlines(compiler);

	int ifJump = emitJump(compiler, CODE_JUMP_IF);

	parsePrecedence(compiler, PREC_CONDITIONAL);
	consume(compiler, TOKEN_COLON, "Expect `:` afther then branch of conditional operator.");
	ignoreNewlines(compiler);

	int elseJump = emitJump(compiler, CODE_JUMP);

	patchJump(compiler, ifJump);
	parsePrecedence(compiler, PREC_ASSIGNMENT);
	patchJump(compiler, elseJump);
}

static void infixOp(Compiler* compiler, bool canAssign) {
	GrammarRule* rule = getRule(compiler->parser->previous.type);
	ignoreNewlines(compiler);
	parsePrecedence(compiler, (Precedence)(rule->precedence + 1));

	Signature signature = {rule->name, (int)strlen(rule->name), SIG_METHOD, 1};
	callSignature(compiler, CODE_CALL_0, &signature);
}

static void infixSignature(Compiler* compiler, Signature* signature) {
	signature->type = SIG_METHOD;
	signature->arity = 1;

	consume(compiler, TOKEN_LEFT_PAREN, "Expect `(` after operator name.");
	declareNamedVariable(compiler);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after parameter name.");
}

static void unarySignature(Compiler* compiler, Signature* signature) {
	signature->type = SIG_GETTER;
}

static void mixedSignature(Compiler* compiler, Signature* signature) {
	signature->type = SIG_GETTER;

	if (match(compiler, TOKEN_LEFT_PAREN)) {
		signature->type = SIG_METHOD;
		signature->arity = 1;

		declareNamedVariable(compiler);
		consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after parameter name.");
	}
}

static bool maybeSetter(Compiler* compiler, Signature* signature) {
	if (!match(compiler, TOKEN_EQ))
		return false;

	if (signature->type == SIG_SUBSCRIPT)
		signature->type = SIG_SUBSCRIPT_SETTER;
	else
		signature->type = SIG_SETTER;

	consume(compiler, TOKEN_LEFT_PAREN, "Expect `(` after `=`.");
	declareNamedVariable(compiler);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after parameter name.");

	++signature->arity;
	return true;
}

static void subscriptSignature(Compiler* compiler, Signature* signature) {
	signature->type = SIG_SUBSCRIPT;
	signature->length = 0;
	finishParameterList(compiler, signature);
	consume(compiler, TOKEN_RIGHT_BRACKET, "Expect `]` after parameters.");

	maybeSetter(compiler, signature);
}

static void parameterList(Compiler* compiler, Signature* signature) {
	if (!match(compiler, TOKEN_LEFT_PAREN))
		return;

	signature->type = SIG_METHOD;
	if (match(compiler, TOKEN_RIGHT_PAREN))
		return;

	finishParameterList(compiler, signature);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after parameters.");
}

static void namedSignature(Compiler* compiler, Signature* signature) {
	signature->type = SIG_GETTER;
	if (maybeSetter(compiler, signature))
		return;

	parameterList(compiler, signature);
}

static void constructorSignature(Compiler* compiler, Signature* signature) {
	consume(compiler, TOKEN_NAME, "Expect constructor name after `construct`.");

	*signature = signatureFromToken(compiler, SIG_INITIALIZER);
	if (match(compiler, TOKEN_EQ))
		error(compiler, "A constructor cannot be a setter.");
	if (!match(compiler, TOKEN_LEFT_PAREN)) {
		error(compiler, "A constructor cannot be a getter.");
		return;
	}

	if (match(compiler, TOKEN_RIGHT_PAREN))
		return;
	finishParameterList(compiler, signature);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after parameters.");
}

static GrammarRule rules[] = {
	RULE_PREFIX(grouping),                                      // WREN_PUNCTUATOR(LEFT_PAREN, "(")
	RULE_UNUSED,                                                // WREN_PUNCTUATOR(RIGHT_PAREN, ")")
	{list, subscript, subscriptSignature, PREC_CALL, NULL},     // WREN_PUNCTUATOR(LEFT_BRACKET, "[")
	RULE_UNUSED,                                                // WREN_PUNCTUATOR(RIGHT_BRACKET, "]")
	RULE_PREFIX(map),                                           // WREN_PUNCTUATOR(LEFT_BRACE, "{")
	RULE_UNUSED,                                                // WREN_PUNCTUATOR(RIGHT_BRACE, "}")
	RULE_UNUSED,                                                // WREN_PUNCTUATOR(COLON, ":")
	RULE_INFIX(PREC_CALL, call),                                // WREN_PUNCTUATOR(DOT, ".")
	RULE_INFIX_OPERATOR(PREC_RANGE, ".."),                      // WREN_PUNCTUATOR(DOTDOT, "..")
	RULE_INFIX_OPERATOR(PREC_RANGE, "..."),                     // WREN_PUNCTUATOR(DOTDOTDOT, "...")
	RULE_UNUSED,                                                // WREN_PUNCTUATOR(COMMA, ",")
	RULE_INFIX_OPERATOR(PREC_FACTOR, "*"),                      // WREN_PUNCTUATOR(STAR, "*")
	RULE_INFIX_OPERATOR(PREC_FACTOR, "/"),                      // WREN_PUNCTUATOR(SLASH, "/")
	RULE_INFIX_OPERATOR(PREC_FACTOR, "%"),                      // WREN_PUNCTUATOR(PERCENT, "%")
	RULE_INFIX_OPERATOR(PREC_TERM, "+"),                        // WREN_PUNCTUATOR(PLUS, "+")
	RULE_OPERATOR("-"),                                         // WREN_PUNCTUATOR(MINUS, "-")
	RULE_INFIX_OPERATOR(PREC_BITWISE_SHIFT, "<<"),              // WREN_PUNCTUATOR(LTLT, "<<")
	RULE_INFIX_OPERATOR(PREC_BITWISE_SHIFT, ">>"),              // WREN_PUNCTUATOR(GTGT, ">>")
	RULE_INFIX_OPERATOR(PREC_BITWISE_OR, "|"),                  // WREN_PUNCTUATOR(PIPE, "|")
	RULE_INFIX(PREC_LOGICAL_OR, or_),                           // WREN_PUNCTUATOR(PIPEPIPE, "||")
	RULE_INFIX_OPERATOR(PREC_BITWISE_XOR, "^"),                 // WREN_PUNCTUATOR(CARET, "^")
	RULE_INFIX_OPERATOR(PREC_BITWISE_AND, "&"),                 // WREN_PUNCTUATOR(AMP, "&")
	RULE_INFIX(PREC_LOGICAL_AND, and_),                         // WREN_PUNCTUATOR(AMPAMP, "&&")
	RULE_PREFIX_OPERATOR("!"),                                  // WREN_PUNCTUATOR(BANG, "!")
	RULE_PREFIX_OPERATOR("~"),                                  // WREN_PUNCTUATOR(TILDE, "~")
	RULE_INFIX(PREC_ASSIGNMENT, conditional),                   // WREN_PUNCTUATOR(QUESTION, "?")
	RULE_UNUSED,                                                // WREN_PUNCTUATOR(EQ, "=")
	RULE_INFIX_OPERATOR(PREC_COMPARISON, "<"),                  // WREN_PUNCTUATOR(LT, "<")
	RULE_INFIX_OPERATOR(PREC_COMPARISON, ">"),                  // WREN_PUNCTUATOR(GT, ">")
	RULE_INFIX_OPERATOR(PREC_COMPARISON, "<="),                 // WREN_PUNCTUATOR(LTEQ, "<=")
	RULE_INFIX_OPERATOR(PREC_COMPARISON, ">="),                 // WREN_PUNCTUATOR(GTEQ, ">=")
	RULE_INFIX_OPERATOR(PREC_EQUALITY, "=="),                   // WREN_PUNCTUATOR(EQEQ, "==")
	RULE_INFIX_OPERATOR(PREC_EQUALITY, "!="),                   // WREN_PUNCTUATOR(BANGEQ, "!=")

	RULE_UNUSED,                                                // WREN_KEYWORD(BREAK, "break")
	RULE_UNUSED,                                                // WREN_KEYWORD(CLASS, "class")
	{NULL, NULL, constructorSignature, PREC_NONE, NULL},        // WREN_KEYWORD(CONSTRUCT, "construct")
	RULE_UNUSED,                                                // WREN_KEYWORD(ELSE, "else")
	RULE_PREFIX(boolean),                                       // WREN_KEYWORD(FALSE, "false")
	RULE_UNUSED,                                                // WREN_KEYWORD(FOR, "for")
	RULE_UNUSED,                                                // WREN_KEYWORD(FOREIGN, "foreign")
	RULE_UNUSED,                                                // WREN_KEYWORD(IF, "if")
	RULE_UNUSED,                                                // WREN_KEYWORD(IMPORT, "import")
	RULE_UNUSED,                                                // WREN_KEYWORD(IN, "in")
	RULE_INFIX_OPERATOR(PREC_IS, "is"),                         // WREN_KEYWORD(IS, "is")
	RULE_PREFIX(null),                                          // WREN_KEYWORD(NULL, "null")
	RULE_UNUSED,                                                // WREN_KEYWORD(RETURN, "return")
	RULE_UNUSED,                                                // WREN_KEYWORD(STATIC, "static")
	RULE_PREFIX(super_),                                        // WREN_KEYWORD(SUPER, "super")
	RULE_PREFIX(this_),                                         // WREN_KEYWORD(THIS, "this")
	RULE_PREFIX(boolean),                                       // WREN_KEYWORD(TRUE, "true")
	RULE_UNUSED,                                                // WREN_KEYWORD(VAR, "var")
	RULE_UNUSED,                                                // WREN_KEYWORD(WHILE, "while")

	RULE_PREFIX(field),                                         // WREN_TOKEN(FIELD, "Field")
	RULE_PREFIX(staticField),                                   // WREN_TOKEN(STATIC_FIELD, "StaticField")
	{name, NULL, namedSignature, PREC_NONE, NULL},              // WREN_TOKEN(NAME, "Name")
	RULE_PREFIX(literal),                                       // WREN_TOKEN(NUMBER, "Number")
	RULE_PREFIX(literal),                                       // WREN_TOKEN(STRING, "String")

	RULE_PREFIX(stringInterpolation),                           // WREN_TOKEN(INTERPOLATION, "Interpolation")
	RULE_UNUSED,                                                // WREN_TOKEN(LINE, "Newline")

	RULE_UNUSED,                                                // WREN_TOKEN(ERROR, "Error")
	RULE_UNUSED,                                                // WREN_TOKEN(EOF, "Eof")
};

static GrammarRule* getRule(TokenType type) {
	return &rules[type];
}

static void parsePrecedence(Compiler* compiler, Precedence precedence) {
	nextToken(compiler->parser);
	GrammarFn prefix = getRule(compiler->parser->previous.type)->prefix;
	if (NULL == prefix) {
		error(compiler, "Expected expression.");
		return;
	}

	bool canAssign = precedence <= PREC_CONDITIONAL;
	prefix(compiler, canAssign);

	while (precedence <= getRule(compiler->parser->current.type)->precedence) {
		nextToken(compiler->parser);
		GrammarFn infix = getRule(compiler->parser->previous.type)->infix;

		if (NULL != infix)
			infix(compiler, canAssign);
	}
}

static void expression(Compiler* compiler) {
	parsePrecedence(compiler, PREC_LOWEST);
}

static int getNumArguments(const u8_t* bytecode, const Value* constants, int ip) {
	Code instruction = (Code)bytecode[ip];
	switch (instruction) {
	case CODE_NULL:
	case CODE_FALSE:
	case CODE_TRUE:
	case CODE_POP:
	case CODE_CLOSE_UPVALUE:
	case CODE_RETURN:
	case CODE_END:
	case CODE_LOAD_LOCAL_0:
	case CODE_LOAD_LOCAL_1:
	case CODE_LOAD_LOCAL_2:
	case CODE_LOAD_LOCAL_3:
	case CODE_LOAD_LOCAL_4:
	case CODE_LOAD_LOCAL_5:
	case CODE_LOAD_LOCAL_6:
	case CODE_LOAD_LOCAL_7:
	case CODE_LOAD_LOCAL_8:
	case CODE_CONSTRUCT:
	case CODE_FOREIGN_CONSTRUCT:
	case CODE_FOREIGN_CLASS:
	case CODE_END_MODULE:
		return 0;

	case CODE_LOAD_LOCAL:
	case CODE_STORE_LOCAL:
	case CODE_LOAD_UPVALUE:
	case CODE_STORE_UPVALUE:
	case CODE_LOAD_FIELD_THIS:
	case CODE_STORE_FIELD_THIS:
	case CODE_LOAD_FIELD:
	case CODE_STORE_FIELD:
	case CODE_CLASS:
		return 1;

	case CODE_CONSTANT:
	case CODE_LOAD_MODULE_VAR:
	case CODE_STORE_MODULE_VAR:
	case CODE_CALL_0:
	case CODE_CALL_1:
	case CODE_CALL_2:
	case CODE_CALL_3:
	case CODE_CALL_4:
	case CODE_CALL_5:
	case CODE_CALL_6:
	case CODE_CALL_7:
	case CODE_CALL_8:
	case CODE_CALL_9:
	case CODE_CALL_10:
	case CODE_CALL_11:
	case CODE_CALL_12:
	case CODE_CALL_13:
	case CODE_CALL_14:
	case CODE_CALL_15:
	case CODE_CALL_16:
	case CODE_JUMP:
	case CODE_LOOP:
	case CODE_JUMP_IF:
	case CODE_AND:
	case CODE_OR:
	case CODE_METHOD_INSTANCE:
	case CODE_METHOD_STATIC:
	case CODE_IMPORT_MODULE:
		return 2;

	case CODE_SUPER_0:
	case CODE_SUPER_1:
	case CODE_SUPER_2:
	case CODE_SUPER_3:
	case CODE_SUPER_4:
	case CODE_SUPER_5:
	case CODE_SUPER_6:
	case CODE_SUPER_7:
	case CODE_SUPER_8:
	case CODE_SUPER_9:
	case CODE_SUPER_10:
	case CODE_SUPER_11:
	case CODE_SUPER_12:
	case CODE_SUPER_13:
	case CODE_SUPER_14:
	case CODE_SUPER_15:
	case CODE_SUPER_16:
	case CODE_IMPORT_VARIABLE:
		return 4;

	case CODE_CLOSURE:
		{
			int constant = (bytecode[ip + 1] << 8) | bytecode[ip + 2];
			ObjFn* loadedFn = AS_FN(constants[constant]);

			return 2 + (loadedFn->numUpvalues * 2);
		} break;
	default: break;
	}

	UNREACHABLE();
	return 0;
}

static void startLoop(Compiler* compiler, Loop* loop) {
	loop->enclosing = compiler->loop;
	loop->start = compiler->fn->code.count - 1;
	loop->scopeDepth = compiler->scopeDepth;
	compiler->loop = loop;
}

static void testExitLoop(Compiler* compiler) {
	compiler->loop->exitJump = emitJump(compiler, CODE_JUMP_IF);
}

static void loopBody(Compiler* compiler) {
	compiler->loop->body = compiler->fn->code.count;
	statement(compiler);
}

static void endLoop(Compiler* compiler) {
	int loopOffset = compiler->fn->code.count - compiler->loop->start + 2;
	emitShortArg(compiler, CODE_LOOP, loopOffset);

	patchJump(compiler, compiler->loop->exitJump);

	int i = compiler->loop->body;
	while (i < compiler->fn->code.count) {
		if (compiler->fn->code.data[i] == CODE_END) {
			compiler->fn->code.data[i] = CODE_JUMP;
			patchJump(compiler, i + 1);
			i += 3;
		}
		else {
			i += 1 + getNumArguments(compiler->fn->code.data, compiler->fn->constants.data, i);
		}
	}

	compiler->loop = compiler->loop->enclosing;
}

static void forStatement(Compiler* compiler) {
	pushScope(compiler);

	consume(compiler, TOKEN_LEFT_PAREN, "Expect `(` after `for`.");
	consume(compiler, TOKEN_NAME, "Expect for loop variable name.");

	const char* name = compiler->parser->previous.start;
	int length = compiler->parser->previous.length;

	consume(compiler, KEYWORD_IN, "Expect `in` after loop variable.");
	ignoreNewlines(compiler);

	expression(compiler);
	int seqSlot = addLocal(compiler, "seq ", 4);

	null(compiler, false);
	int iterSlot = addLocal(compiler, "iter ", 5);

	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after loop expression.");

	Loop loop;
	startLoop(compiler, &loop);

	loadLocal(compiler, seqSlot);
	loadLocal(compiler, iterSlot);
	callMethod(compiler, 1, "iterate(_)", 10);
	emitByteArg(compiler, CODE_STORE_LOCAL, iterSlot);
	testExitLoop(compiler);

	loadLocal(compiler, seqSlot);
	loadLocal(compiler, iterSlot);
	callMethod(compiler, 1, "iteratorValue(_)", 16);

	pushScope(compiler);
	addLocal(compiler, name, length);
	loopBody(compiler);
	popScope(compiler);

	endLoop(compiler);
	popScope(compiler);
}

static void ifStatement(Compiler* compiler) {
	consume(compiler, TOKEN_LEFT_PAREN, "Expect `(` after `if`.");
	expression(compiler);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after if condition.");

	int ifJump = emitJump(compiler, CODE_JUMP_IF);
	statement(compiler);

	if (match(compiler, KEYWORD_ELSE)) {
		int elseJump = emitJump(compiler, CODE_JUMP);
		patchJump(compiler, ifJump);

		statement(compiler);
		patchJump(compiler, elseJump);
	}
	else {
		patchJump(compiler, ifJump);
	}
}

static void whileStatement(Compiler* compiler) {
	Loop loop;
	startLoop(compiler, &loop);

	consume(compiler, TOKEN_LEFT_PAREN, "Expect `(` after `while`.");
	expression(compiler);
	consume(compiler, TOKEN_RIGHT_PAREN, "Expect `)` after while condition.");

	testExitLoop(compiler);
	loopBody(compiler);
	endLoop(compiler);
}

static void statement(Compiler* compiler) {
	if (match(compiler, KEYWORD_BREAK)) {
		if (NULL == compiler->loop) {
			error(compiler, "Cannot use `break` outside of a loop.");
			return;
		}

		discardLocals(compiler, compiler->loop->scopeDepth + 1);
		emitJump(compiler, CODE_END);
	}
	else if (match(compiler, KEYWORD_FOR)) {
		forStatement(compiler);
	}
	else if (match(compiler, KEYWORD_IF)) {
		ifStatement(compiler);
	}
	else if (match(compiler, KEYWORD_RETURN)) {
		if (peek(compiler) == TOKEN_LINE)
			emitOp(compiler, CODE_NULL);
		else
			expression(compiler);

		emitOp(compiler, CODE_RETURN);
	}
	else if (match(compiler, KEYWORD_WHILE)) {
		whileStatement(compiler);
	}
	else if (match(compiler, TOKEN_LEFT_BRACE)) {
		pushScope(compiler);
		if (finishBlock(compiler))
			emitOp(compiler, CODE_POP);
		popScope(compiler);
	}
	else {
		expression(compiler);
		emitOp(compiler, CODE_POP);
	}
}

static void createConstructor(Compiler* compiler, Signature* signature, int initializerSymbol) {
	Compiler methodCompiler;
	initCompiler(&methodCompiler, compiler->parser, compiler, true);

	emitOp(&methodCompiler, compiler->enclosingClass->isForeign ? CODE_FOREIGN_CONSTRUCT : CODE_CONSTRUCT);
	emitShortArg(&methodCompiler, (Code)(CODE_CALL_0 + signature->arity), initializerSymbol);

	emitOp(&methodCompiler, CODE_RETURN);
	endCompiler(&methodCompiler, "", 0);
}

static void defineMethod(Compiler* compiler, Variable classVariable, bool isStatic, int methodSymbol) {
	loadVariable(compiler, classVariable);
	Code instruction = isStatic ? CODE_METHOD_STATIC : CODE_METHOD_INSTANCE;
	emitShortArg(compiler, instruction, methodSymbol);
}

static int declareMethod(Compiler* compiler, Signature* signature, const char* name, int length) {
	int symbol = signatureSymbol(compiler, signature);
	ClassInfo* classInfo = compiler->enclosingClass;
	IntBuffer* methods = classInfo->inStatic ? &classInfo->staticMethods : &classInfo->methods;
	for (int i = 0; i < methods->count; ++i) {
		if (methods->data[i] == symbol) {
			const char* staticPrefix = classInfo->inStatic ? "static " : "";
			error(compiler, "Class %s already defines a %smethod `%s`.",
					&compiler->enclosingClass->name->value, staticPrefix, name);
			break;
		}
	}

	wrenIntBufferWrite(compiler->parser->vm, methods, symbol);
	return symbol;
}

static bool method(Compiler* compiler, Variable classVariable) {
	bool isForeign = match(compiler, KEYWORD_FOREIGN);
	bool isStatic = match(compiler, KEYWORD_STATIC);
	compiler->enclosingClass->inStatic = isStatic;

	SignatureFn signatureFn = getRule(compiler->parser->current.type)->method;
	nextToken(compiler->parser);

	if (NULL == signatureFn) {
		error(compiler, "Expect method definition.");
		return false;
	}

	Signature signature = signatureFromToken(compiler, SIG_GETTER);
	compiler->enclosingClass->signature = &signature;

	Compiler methodCompiler;
	initCompiler(&methodCompiler, compiler->parser, compiler, true);

	signatureFn(&methodCompiler, &signature);
	if (isStatic && signature.type == SIG_INITIALIZER)
		error(compiler, "A constructor cannot be static.");

	char fullSignature[WREN_MAX_METHOD_SIGNATURE];
	int length;
	signatureToString(&signature, fullSignature, &length);

	int methodSymbol = declareMethod(compiler, &signature, fullSignature, length);
	if (isForeign) {
		emitConstant(compiler, wrenNewStringLength(compiler->parser->vm, fullSignature, length));
		methodCompiler.parser->vm->compiler = methodCompiler.parent;
	}
	else {
		consume(compiler, TOKEN_LEFT_BRACE, "Expect `{` to begin method body.");
		finishBody(&methodCompiler, signature.type == SIG_INITIALIZER);
		endCompiler(&methodCompiler, fullSignature, length);
	}

	defineMethod(compiler, classVariable, isStatic, methodSymbol);
	if (signature.type == SIG_INITIALIZER) {
		signature.type = SIG_METHOD;
		int constructorSymbol = signatureSymbol(compiler, &signature);

		createConstructor(compiler, &signature, methodSymbol);
		defineMethod(compiler, classVariable, true, constructorSymbol);
	}
	return true;
}

static void classDefinition(Compiler* compiler, bool isForeign) {
	Variable classVariable;
	classVariable.scope = compiler->scopeDepth == -1 ? SCOPE_MODULE : SCOPE_LOCAL;
	classVariable.index = declareNamedVariable(compiler);

	Value classNameString = wrenNewStringLength(compiler->parser->vm,
			compiler->parser->previous.start, compiler->parser->previous.length);
	ObjString* className = AS_STRING(classNameString);

	emitConstant(compiler, classNameString);

	if (match(compiler, KEYWORD_IS))
		parsePrecedence(compiler, PREC_CALL);
	else
		loadCoreVariable(compiler, "Object");

	int numFieldsInstruction = -1;
	if (isForeign)
		emitOp(compiler, CODE_FOREIGN_CLASS);
	else
		numFieldsInstruction = emitByteArg(compiler, CODE_CLASS, 255);

	defineVariable(compiler, classVariable.index);

	pushScope(compiler);
	ClassInfo classInfo;
	classInfo.isForeign = isForeign;
	classInfo.name = className;

	wrenSymbolTableInit(&classInfo.fields);
	wrenIntBufferInit(&classInfo.methods);
	wrenIntBufferInit(&classInfo.staticMethods);
	compiler->enclosingClass = &classInfo;

	consume(compiler, TOKEN_LEFT_BRACE, "Expect `{` after class declaration.");
	matchLine(compiler);

	while (!match(compiler, TOKEN_RIGHT_BRACE)) {
		if (!method(compiler, classVariable))
			break;
		if (match(compiler, TOKEN_RIGHT_BRACE))
			break;
		consumeLine(compiler, "Expect newline after definition in class.");
	}

	if (!isForeign)
		compiler->fn->code.data[numFieldsInstruction] = (u8_t)classInfo.fields.count;

	wrenSymbolTableClear(compiler->parser->vm, &classInfo.fields);
	wrenIntBufferClear(compiler->parser->vm, &classInfo.methods);
	wrenIntBufferClear(compiler->parser->vm, &classInfo.staticMethods);
	compiler->enclosingClass = NULL;

	popScope(compiler);
}

static void import(Compiler* compiler) {
	ignoreNewlines(compiler);
	consume(compiler, TOKEN_STRING, "Expect a string after `import`.");
	int moduleConstant = addConstant(compiler, compiler->parser->previous.value);

	emitShortArg(compiler, CODE_IMPORT_MODULE, moduleConstant);
	emitOp(compiler, CODE_POP);

	if (!match(compiler, KEYWORD_FOR))
		return;

	do {
		ignoreNewlines(compiler);
		int slot = declareNamedVariable(compiler);
		int variableConstant = addConstant(compiler, wrenNewStringLength(
					compiler->parser->vm, compiler->parser->previous.start, compiler->parser->previous.length));

		emitShortArg(compiler, CODE_IMPORT_VARIABLE, variableConstant);
		defineVariable(compiler, slot);
	} while (match(compiler, TOKEN_COMMA));
}

static void variableDefinition(Compiler* compiler) {
	consume(compiler, TOKEN_NAME, "Expect variable name.");
	Token nameToken = compiler->parser->previous;

	if (match(compiler, TOKEN_EQ)) {
		ignoreNewlines(compiler);
		expression(compiler);
	}
	else {
		null(compiler, false);
	}

	int symbol = declareVariable(compiler, &nameToken);
	defineVariable(compiler, symbol);
}

static void definition(Compiler* compiler) {
	if (match(compiler, KEYWORD_CLASS)) {
		classDefinition(compiler, false);
	}
	else if (match(compiler, KEYWORD_FOREIGN)) {
		consume(compiler, KEYWORD_CLASS, "Expect `class` after `foreign`.");
		classDefinition(compiler, true);
	}
	else if (match(compiler, KEYWORD_IMPORT)) {
		import(compiler);
	}
	else if (match(compiler, KEYWORD_VAR)) {
		variableDefinition(compiler);
	}
	else {
		statement(compiler);
	}
}

ObjFn* wrenCompile(WrenVM* vm,
		ObjModule* module, const char* sourceCode, bool isExpression, bool printErrors) {
	if (strncmp(sourceCode, WREN_UTF8BOM, WREN_UTF8BOM_LEN) == 0)
		sourceCode += WREN_UTF8BOM_LEN;

	Parser parser;
	parser.vm = vm;
	parser.module = module;
	parser.sourceCode = sourceCode;
	parser.tokenStart = sourceCode;
	parser.currentChar = sourceCode;
	parser.currentLine = 1;
	parser.numParens = 0;

	parser.current.type = TOKEN_ERROR;
	parser.current.start = sourceCode;
	parser.current.length = 0;
	parser.current.lineno = 0;
	parser.current.value = UNDEFINED_VAL;

	parser.skipNewlines = true;
	parser.printErrors = printErrors;
	parser.hasError = false;

	nextToken(&parser);
	int numExistingVariables = module->variables.count;

	Compiler compiler;
	initCompiler(&compiler, &parser, NULL, false);
	ignoreNewlines(&compiler);

	if (isExpression) {
		expression(&compiler);
		consume(&compiler, TOKEN_EOF, "Expect end of expression.");
	}
	else {
		while (!match(&compiler, TOKEN_EOF)) {
			definition(&compiler);

			if (!matchLine(&compiler)) {
				consume(&compiler, TOKEN_EOF, "Expect end of file.");
				break;
			}
		}
		emitOp(&compiler, CODE_END_MODULE);
	}
	emitOp(&compiler, CODE_RETURN);

	for (int i = numExistingVariables; i < parser.module->variables.count; ++i) {
		if (IS_NUM(parser.module->variables.data[i])) {
			parser.previous.type = TOKEN_NAME;
			parser.previous.start = parser.module->variableNames.data[i]->value;
			parser.previous.length = parser.module->variableNames.data[i]->length;
			parser.previous.lineno = (int)AS_NUM(parser.module->variables.data[i]);
			error(&compiler, "Variable is used but not defined.");
		}
	}

	return endCompiler(&compiler, "(script)", 8);
}

void wrenBindMethodCode(ObjClass* classObj, ObjFn* fn) {
	int ip = 0;
	for (;;) {
		Code instruction = (Code)fn->code.data[ip];
		switch (instruction) {
		case CODE_LOAD_FIELD:
		case CODE_STORE_FIELD:
		case CODE_LOAD_FIELD_THIS:
		case CODE_STORE_FIELD_THIS:
			fn->code.data[ip + 1] += classObj->superclass->numFields;
			break;

		case CODE_SUPER_0:
		case CODE_SUPER_1:
		case CODE_SUPER_2:
		case CODE_SUPER_3:
		case CODE_SUPER_4:
		case CODE_SUPER_5:
		case CODE_SUPER_6:
		case CODE_SUPER_7:
		case CODE_SUPER_8:
		case CODE_SUPER_9:
		case CODE_SUPER_10:
		case CODE_SUPER_11:
		case CODE_SUPER_12:
		case CODE_SUPER_13:
		case CODE_SUPER_14:
		case CODE_SUPER_15:
		case CODE_SUPER_16:
			{
				int constant = (fn->code.data[ip + 3] << 8) | fn->code.data[ip + 4];
				fn->constants.data[constant] = OBJ_VAL(classObj->superclass);
			} break;

		case CODE_CLOSURE:
			{
				int constant = (fn->code.data[ip + 1] << 8) | fn->code.data[ip + 2];
				wrenBindMethodCode(classObj, AS_FN(fn->constants.data[constant]));
			} break;

		case CODE_END: return;
		default: break;
		}

		ip += 1 + getNumArguments(fn->code.data, fn->constants.data, ip);
	}
}

void wrenMarkCompiler(WrenVM* vm, Compiler* compiler) {
	wrenGrayValue(vm, compiler->parser->current.value);
	wrenGrayValue(vm, compiler->parser->previous.value);

	do {
		wrenGrayObj(vm, (Obj*)compiler->fn);
		wrenGrayObj(vm, (Obj*)compiler->constants);

		if (NULL != compiler->enclosingClass)
			wrenBlackenSymbolTable(vm, &compiler->enclosingClass->fields);

		compiler = compiler->parent;
	} while (NULL != compiler);
}
