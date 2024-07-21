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

typedef struct {
  char*                 buffer;
  sz_t                  buffer_length;
  ssz_t                 input_length;
} InputBuffer;

static ssz_t getline(char** lineptr, sz_t* length, FILE* stream) {
  char* buffer = *lineptr;
  int c;
  ssz_t n = 0;

  if (NULL == buffer || 0 == *length) {
    *length = 16;
    *lineptr = (char*)malloc(*length);
    buffer = *lineptr;
  }

  while ('\n' != (c = fgetc(stream))) {
    if (EOF == c)
      return -1;

    if ((sz_t)n < *length - 2) {
      *(buffer + n++) = c;
    }
    else {
      *length = *length + 16;
      buffer = (char*)realloc(buffer, *length);
      *(buffer + n++) = c;
    }
  }

  *(buffer + n++) = '\n';
  *(buffer + n) = 0;
  return n;
}

InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer            = NULL;
  input_buffer->buffer_length     = 0;
  input_buffer->input_length      = 0;

  return input_buffer;
}

void close_input_buffer(InputBuffer* input_buffer) {
  if (NULL != input_buffer) {
    if (NULL != input_buffer->buffer)
      free(input_buffer->buffer);
    free(input_buffer);
  }
}

void read_input(InputBuffer* input_buffer) {
  ssz_t bytes_read = getline(&input_buffer->buffer, &input_buffer->buffer_length, stdin);

  if (bytes_read <= 0) {
    fprintf(stderr, "Error reading input\n");
    exit(EXIT_FAILURE);
  }

  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void print_prompt() {
  fprintf(stdout, "db > ");
}

int main(int argc, char* argv[]) {
  SDB_UNUSED(argc), SDB_UNUSED(argv);

  InputBuffer* input_buffer = new_input_buffer();
  for (;;) {
    print_prompt();
    read_input(input_buffer);

    if (0 == strcmp(input_buffer->buffer, ".exit")) {
      close_input_buffer(input_buffer);
      exit(EXIT_SUCCESS);
    }
    else {
      fprintf(stderr, "Unrecognized command `%s`\n", input_buffer->buffer);
    }
  }

  return 0;
}
