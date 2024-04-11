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
#ifndef LOXC_OBJECT_H
#define LOXC_OBJECT_H

#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value)                     (AS_OBJ(value)->type)

#define IS_BOUND_METHOD(value)              isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value)                     isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)                   isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)                  isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value)                  isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)                    isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)                    isObjType(value, OBJ_STRING)
#define IS_UPVALUE(value)                   isObjType(value, OBJ_UPVALUE)

#define AS_BOUND_METHOD(value)              ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)                     ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)                   ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value)                  ((ObjFunction*)AS_OBJ(value))
#define AS_INSTANCE(value)                  ((ObjInstance*)AS_OBJ(value))
#define AS_NATIVE(value)                    (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)                    ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)                   (((ObjString*)AS_OBJ(value))->chars)
#define AS_UPVALUE(value)                   ((ObjUpvalue*)AS_OBJ(value))

typedef enum {
  OBJ_BOUND_METHOD,
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_UPVALUE,
} ObjType;

struct Obj {
  ObjType               type;
  bool                  isMarked;
  struct Obj*           next;
};

typedef struct {
  Obj                   obj;

  int                   arity;
  int                   upvalueCount;
  Chunk                 chunk;
  ObjString*            name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
  Obj                   obj;

  NativeFn              function;
} ObjNative;

struct ObjString {
  Obj                   obj;

  int                   length;
  char*                 chars;
  u32_t                 hash;
};

typedef struct ObjUpvalue {
  Obj                   obj;

  Value*                location;
  Value                 closed;
  struct ObjUpvalue*    next;
} ObjUpvalue;

typedef struct {
  Obj                   obj;

  ObjFunction*          function;
  ObjUpvalue**          upvalues;
  int                   upvalueCount;
} ObjClosure;

typedef struct {
  Obj                   obj;

  ObjString*            name;
  Table                 methods;
} ObjClass;

typedef struct {
  Obj                   obj;

  ObjClass*             klass;
  Table                 fields;
} ObjInstance;

typedef struct {
  Obj                   obj;

  Value                 receiver;
  ObjClosure*           method;
} ObjBoundMethod;

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name);
ObjClosure* newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjInstance* newInstance(ObjClass* klass);
ObjNative* newNative(NativeFn function);
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
ObjUpvalue* newUpvalue(Value* value);

void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
