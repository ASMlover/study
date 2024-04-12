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
#ifndef LOXC_VALUE_H
#define LOXC_VALUE_H

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
} ValueType;

typedef struct {
  ValueType             type;
  union {
    bool                boolean;
    double              number;
    Obj*                obj;
  } as;
} Value;

#define IS_BOOL(value)                      ((value).type == VAL_BOOL)
#define IS_NIL(value)                       ((value).type == VAL_NIL)
#define IS_NUMBER(value)                    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)                       ((value).type == VAL_OBJ)

#define AS_BOOL(value)                      ((value).as.boolean)
#define AS_NUMBER(value)                    ((value).as.number)
#define AS_OBJ(value)                       ((value).as.obj)

#define BOOL_VAL(value)                     ((Value){VAL_BOOL, {.boolean = (value)}})
#define NIL_VAL                             ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)                   ((Value){VAL_NUMBER, {.number = (value)}})
#define OBJ_VAL(object)                     ((Value){VAL_OBJ, {.obj = (Obj*)(object)}})

typedef struct {
  int                   count;
  int                   capacity;
  Value*                values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);

bool valuesEqual(Value a, Value b);
void printValue(Value value);

#endif
