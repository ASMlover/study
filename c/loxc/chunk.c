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
#include "memory.h"
#include "chunk.h"
#include "vm.h"

static int constantInstruction(Chunk* chunk, const char* name, int offset) {
  u8_t constant = chunk->code[offset + 1];
  fprintf(stdout, "%-16s %4d `", name, constant);
  printValue(chunk->constants.values[constant]);
  fprintf(stdout, "`\n");

  return offset + 2;
}

static int invokeInstruction(Chunk* chunk, const char* name, int offset) {
  u8_t constant = chunk->code[offset + 1];
  u8_t argCount = chunk->code[offset + 2];
  fprintf(stdout, "%-16s (%d args) %4d `", name, argCount, constant);
  printValue(chunk->constants.values[constant]);
  fprintf(stdout, "`\n");

  return offset + 3;
}

static inline simpleInstruction(const char* name, int offset) {
  fprintf(stdout, "%s\n", name);
  return offset + 1;
}

static int jumpInstruction(Chunk* chunk, const char* name, int sign, int offset) {
  u16_t jump = (u16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  fprintf(stdout, "%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);

  return offset + 3;
}

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(u8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

void writeChunk(Chunk* chunk, u8_t byte, int lineno) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    int newCapacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(u8_t, chunk->code, oldCapacity, newCapacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, newCapacity);
    chunk->capacity = newCapacity;
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = lineno;
  ++chunk->count;
}

void addConstant(Chunk* chunk, Value value) {
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();
}

void disassembleChunk(Chunk* chunk, const char* name) {
  fprintf(stdout, "========= %s =========\n", name);

  for (int offset = 0; offset < chunk->count;)
    offset = disassebleInstruction(chunk, offset);
}

int disassebleInstruction(Chunk* chunk, int offset) {
  fprintf(stdout, "%04d ", offset);
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    fprintf(stdout, "   | ");
  else
    fprintf(stdout, "%04d ", chunk->lines[offset]);

  u8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_CONSTANT: return constantInstruction(chunk, "OP_CONSTANT", offset);
  default: fprintf(stdout, "Unknown opcode %d\n", instruction); break;
  }

  return offset + 1;
}
