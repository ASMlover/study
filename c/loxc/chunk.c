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

static inline int constantInstruction(Chunk* chunk, const char* name, int offset) {
  u8_t constant = chunk->code[offset + 1];
  fprintf(stdout, "%-16s %4d `", name, constant);
  printValue(chunk->constants.values[constant]);
  fprintf(stdout, "`\n");

  return offset + 2;
}

static inline int invokeInstruction(Chunk* chunk, const char* name, int offset) {
  u8_t constant = chunk->code[offset + 1];
  u8_t argCount = chunk->code[offset + 2];
  fprintf(stdout, "%-16s (%d args) %4d `", name, argCount, constant);
  printValue(chunk->constants.values[constant]);
  fprintf(stdout, "`\n");

  return offset + 3;
}

static inline int simpleInstruction(const char* name, int offset) {
  fprintf(stdout, "%s\n", name);
  return offset + 1;
}

static inline int byteInstruction(Chunk* chunk, const char* name, int offset) {
  u8_t slot = chunk->code[offset + 1];
  fprintf(stdout, "%-16s %4d\n", name, slot);
  return offset + 2;
}

static inline int jumpInstruction(Chunk* chunk, const char* name, int sign, int offset) {
  u16_t jump = (u16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  fprintf(stdout, "%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);

  return offset + 3;
}

Chunk* allocChunk() {
  Chunk* chunk = ALLOCATE(Chunk, 1);
  if (NULL != chunk)
    initChunk(chunk);
  return chunk;
}

void deallocChunk(Chunk* chunk) {
  destroyChunk(chunk);
  FREE(Chunk, chunk);
}

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  initValueArray(&chunk->constants);
}

void destroyChunk(Chunk* chunk) {
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

int addConstant(Chunk* chunk, Value value) {
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();

  return chunk->constants.count - 1;
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
  case OP_CONSTANT:       return constantInstruction(chunk, "OP_CONSTANT", offset);
  case OP_NIL:            return simpleInstruction("OP_NIL", offset);
  case OP_TRUE:           return simpleInstruction("OP_TRUE", offset);
  case OP_FALSE:          return simpleInstruction("OP_FALSE", offset);
  case OP_POP:            return simpleInstruction("OP_POP", offset);
  case OP_GET_LOCAL:      return byteInstruction(chunk, "OP_GET_LOCAL", offset);
  case OP_SET_LOCAL:      return byteInstruction(chunk, "OP_SET_LOCAL", offset);
  case OP_GET_GLOBAL:     return constantInstruction(chunk, "OP_GET_GLOBAL", offset);
  case OP_DEFINE_GLOBAL:  return constantInstruction(chunk, "OP_DEFINE_GLOBAL", offset);
  case OP_SET_GLOBAL:     return constantInstruction(chunk, "OP_SET_GLOBAL", offset);
  case OP_GET_UPVALUE:    return byteInstruction(chunk, "OP_GET_UPVALUE", offset);
  case OP_SET_UPVALUE:    return byteInstruction(chunk, "OP_SET_UPVALUE", offset);
  case OP_GET_PROPERTY:   return constantInstruction(chunk, "OP_GET_PROPERTY", offset);
  case OP_SET_PROPERTY:   return constantInstruction(chunk, "OP_SET_PROPERTY", offset);
  case OP_GET_SUPER:      return constantInstruction(chunk, "OP_GET_SUPER", offset);
  case OP_EQUAL:          return simpleInstruction("OP_EQUAL", offset);
  case OP_GREATER:        return simpleInstruction("OP_GREATER", offset);
  case OP_LESS:           return simpleInstruction("OP_LESS", offset);
  case OP_ADD:            return simpleInstruction("OP_ADD", offset);
  case OP_SUBTRACT:       return simpleInstruction("OP_SUBTRACT", offset);
  case OP_MULTIPLY:       return simpleInstruction("OP_MULTIPLY", offset);
  case OP_DIVIDE:         return simpleInstruction("OP_DIVIDE", offset);
  case OP_NOT:            return simpleInstruction("OP_NOT", offset);
  case OP_NEGATE:         return simpleInstruction("OP_NEGATE", offset);
  case OP_PRINT:          return simpleInstruction("OP_PRINT", offset);
  case OP_JUMP:           return jumpInstruction(chunk, "OP_JUMP", 1, offset);
  case OP_JUMP_IF_FALSE:  return jumpInstruction(chunk, "OP_JUMP_IF_FALSE", 1, offset);
  case OP_LOOP:           return jumpInstruction(chunk, "OP_LOOP", -1, offset);
  case OP_CALL:           return byteInstruction(chunk, "OP_CALL", offset);
  case OP_INVOKE:         return invokeInstruction(chunk, "OP_INVOKE", offset);
  case OP_SUPER_INVOKE:   return invokeInstruction(chunk, "OP_SUPER_INVOKE", offset);
  case OP_CLOSURE:
    {
      ++offset;
      u8_t constant = chunk->code[offset++];
      fprintf(stdout, "%-16s %4d ", "OP_CLOSURE", constant);
      printValue(chunk->constants.values[constant]);
      fprintf(stdout, "\n");

      ObjFunction* function = AS_FUNCTION(chunk->constants.values[constant]);
      for (int j = 0; j < function->upvalueCount; ++j) {
        int isLocal = chunk->code[offset++];
        int index = chunk->code[offset++];
        fprintf(stdout, "%04d      |                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
      }
      return offset;
    } break;
  case OP_CLOSE_UPVALUE:  return simpleInstruction("OP_CLOSE_UPVALUE", offset);
  case OP_RETURN:         return simpleInstruction("OP_RETURN", offset);
  case OP_CLASS:          return constantInstruction(chunk, "OP_CLASS", offset);
  case OP_INHERIT:        return simpleInstruction("OP_INHERIT", offset);
  case OP_METHOD:         return constantInstruction(chunk, "OP_METHOD", offset);
  default:                fprintf(stdout, "Unknown opcode %d\n", instruction); break;
  }

  return offset + 1;
}
