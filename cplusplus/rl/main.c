/*
 * Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "ru.h"
#include "ru_asm.h"
#include "ru_parser.h"

struct {
  uint32_t addr[0xFF];
  int count;
} gMem;

static unsigned int w;
static void _SetXor128(void) {
  w = 1234 + (getpid() ^ 0xFFBA9285);
}

static void _Init(void) {
}

static void _FreeAddr(void) {
}

static void _Dispose(void) {
}

static int32_t _Lex(const char* code) {
  return 0;
}

static void _PutI32(int32_t n) {
  fprintf(stdout, "%d", n);
}

static void _PutStr(int32_t* n) {
  fprintf(stdout, "%s", (const char*)n);
}

static void _PutLn(void) {
  fprintf(stdout, "\n");
}

static void _Sleep(uint32_t t) {
  usleep(t * CLOCKS_PER_SEC / 1000);
}

static void _AddMem(int32_t addr) {
  gMem.addr[gMem.count++] = addr;
}

static int _Xor128(void) {
  return 0;
}

static void* funcTable[] = {
  _PutI32, _PutStr, _PutLn, malloc,
  _Xor128, printf, _AddMem, _Sleep,
  fopen, fprintf, fclose, fgets,
  free, _FreeAddr,
};

static int _Execute(const char* code) {
  return 0;
}

int main(int argc, char* argv[]) {
  return 0;
}
