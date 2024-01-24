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
#include "scanner.h"

void scannerTest() {
	fprintf(stderr, "--------- [scannerTest] Beginning ---------\n");
	FILE* fp = fopen("../test.clox", "rb");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: [scannerTest] open test.clox failed !!! \n");
		return;
	}

	fseek(fp, 0L, SEEK_END);
	size_t fileSize = ftell(fp);
	rewind(fp);

	char* sourceCode = (char*)malloc(fileSize + 1);
	if (sourceCode == NULL)
		goto cleanUp;

	size_t bytesRead = fread(sourceCode, sizeof(char), fileSize, fp);
	if (bytesRead < fileSize)
		goto cleanUp;

	sourceCode[fileSize] = 0;
	initScanner(sourceCode);
	do {
		Token token = scanToken();
		if (token.type == TOKEN_EOF)
			break;

		fprintf(stdout, "Token{type:%s, literal:%.*s, lineno:%d}\n",
				tokenTypeAsString(token.type), token.length, token.start, token.lineno);
	} while (true);
cleanUp:
	if (sourceCode != NULL)
		free(sourceCode);
	if (fp != NULL)
		fclose(fp);
}
