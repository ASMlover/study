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
#include "chunk.h"
#include "vm.h"

static void repl() {
	char line[1024];
	for (;;) {
		fprintf(stdout, ">>> ");

		if (!fgets(line, sizeof(line), stdin)) {
			fprintf(stdout, "\n");
			break;
		}

		interpret(line);
	}
}

static char* readFile(const char* filePath) {
	FILE* file = fopen(filePath, "rb");
	if (file == NULL) {
		fprintf(stderr, "Could not open file `%s`.\n", filePath);
		exit(-1);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read `%s`.\n", filePath);
		exit(-1);
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "Could not read file `%s`.\n", filePath);
		exit(-1);
	}

	buffer[bytesRead] = 0;
	fclose(file);

	return buffer;
}

static void runFile(const char* filePath) {
	char* sourceCode = readFile(filePath);
	InterpretResult result = interpret(sourceCode);
	free(sourceCode);

	if (result == INTERPRET_COMPILE_ERROR)
		exit(-2);
	if (result == INTERPRET_RUNTIME_ERROR)
		exit(-3);
}

int main(int argc, char* argv[]) {
	CLOX_UNUSED(argc), CLOX_UNUSED(argv);

	initVM();
	if (argc == 1) {
		repl();
	}
	else if (argc == 2) {
		runFile(argv[1]);
	}
	else {
		fprintf(stderr, "Usgae: clox [filePath]\n");
		exit(-1);
	}
	freeVM();

	return 0;
}
