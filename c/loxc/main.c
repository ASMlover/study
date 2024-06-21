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

static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
  if (NULL == file) {
    fprintf(stderr, "Could not open file `%s`.\n", path);
    exit(-1);
  }

  fseek(file, 0L, SEEK_END);
  sz_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);
  if (NULL == buffer) {
    fprintf(stderr, "Not enougn memory to read `%s`.\n", path);
    exit(-1);
  }

  sz_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file `%s`.\n", path);
    exit(-1);
  }

  buffer[bytesRead] = 0;

  fclose(file);
  return buffer;
}

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

static void runFile(const char* path) {
  char* sourceCode = readFile(path);
  InterpretResult result = interpret(sourceCode);
  free(sourceCode);

  if (INTERPRET_COMPILE_ERROR == result)
    exit(-1);
  if (INTERPRET_RUNTIME_ERROR == result)
    exit(-1);
}

int main(int argc, char* argv[]) {
  LOXC_UNUSED(argc), LOXC_UNUSED(argv);

  return 0;
}
