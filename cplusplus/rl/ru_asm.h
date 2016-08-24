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
#ifndef __RU_ASM_HEADER_H__
#define __RU_ASM_HEADER_H__

unsigned char* gRuCode;
int gRuCount;

enum {RU_EAX = 0, RU_ECX, RU_EDX, RU_EBX, RU_ESP, RU_EBP, RU_ESI, RU_EDI};

static inline void ru_Emit(unsigned char value) {
  gRuCode[gRuCount++] = value;
}

static inline void ru_EmitI32(unsigned int value) {
  ru_Emit(value << 24 >> 24);
  ru_Emit(value << 16 >> 24);
  ru_Emit(value << 8 >> 24);
  ru_Emit(value << 0 >> 24);
}

static inline void ru_EmitI32Insert(unsigned int value, int pos) {
  gRuCode[pos + 0] = (value << 24 >> 24);
  gRuCode[pos + 1] = (value << 16 >> 24);
  gRuCode[pos + 2] = (value << 8 >> 24);
  gRuCode[pos + 3] = (value << 0 >> 24);
}

#endif /* __RU_ASM_HEADER_H__ */
