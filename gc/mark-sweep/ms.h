/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#pragma once

#include <stdint.h>

#define NyAPI_FUNC(type) extern type

typedef enum _type {
  OBJECT_INT,
  OBJECT_PAIR,
} NyType;

typedef struct _gc {
  uint8_t marked;
} NyGC;

typedef struct _object {
  NyGC gc;

  NyType type;
  struct _object* next;
  union {
    int value;
    struct {
      struct _object* head;
      struct _object* tail;
    };
  };
} NyObject;

typedef struct _vm NyVM;

NyAPI_FUNC(NyVM*) NyVM_New(void);
NyAPI_FUNC(void) NyVM_Free(NyVM* vm);
NyAPI_FUNC(NyObject*) NyObject_PushInt(NyVM* vm, int value);
NyAPI_FUNC(NyObject*) NyObject_PushPair(NyVM* vm);
NyAPI_FUNC(NyObject*) NyObject_Pop(NyVM* vm);
NyAPI_FUNC(void) NyGC_Collect(NyVM* vm);
