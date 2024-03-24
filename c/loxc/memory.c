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
#include <stdlib.h>
#include "memory.h"
#include "value.h"
#include "vm.h"

#define LOXC_GC_HEAP_GROW_FACTOR            (2)

static inline void markArray(ValueArray* array) {
  for (int i = 0; i < array->count; ++i)
    MARK_VAL(array->values[i]);
}

static inline void blackenBoundMethod(ObjBoundMethod* bound) {
  MARK_VAL(bound->receiver);
  MARK_OBJ(bound->method);
}

static inline void blackenClass(ObjClass* klass) {
  MARK_OBJ(klass->name);
  markTable(&klass->methods);
}

static inline void blackenClosure(ObjClosure* closure) {
  MARK_OBJ(closure->function);
  for (int i = 0; i < closure->upvalueCount; ++i)
    MARK_OBJ(closure->upvalues[i]);
}

static inline void blackenFunction(ObjFunction* function) {
  MARK_OBJ(function->name);
  markArray(&function->chunk.constants);
}

static inline void blackenInstance(ObjInstance* instance) {
  MARK_OBJ(instance->klass);
  markTable(&instance->fields);
}

static inline void blackenUpvalue(ObjUpvalue* upvalue) {
  MARK_VAL(upvalue->closed);
}

static void blackenObject(Obj* object) {
#if defined(LOXC_DEBUG_LOG_GC)
  fprintf(stdout, "%p blacken ", (void*)object);
  printValue(OBJ_VAL(object));
  fprintf(stdout, "\n");
#endif

  switch (object->type) {
  case OBJ_BOUND_METHOD:  blackenBoundMethod((ObjBoundMethod*)object); break;
  case OBJ_CLASS:         blackenClass((ObjClass*)object); break;
  case OBJ_CLOSURE:       blackenClosure((ObjClosure*)object); break;
  case OBJ_FUNCTION:      blackenFunction((ObjFunction*)object); break;
  case OBJ_INSTANCE:      blackenInstance((ObjInstance*)object); break;
  case OBJ_UPVALUE:       blackenUpvalue((ObjUpvalue*)object); break;
  case OBJ_NATIVE:
  case OBJ_STRING:
  default: break;
  }
}

static inline void freeBoundMethod(Obj* object) {
  FREE(ObjBoundMethod, object);
}

static void freeObject(Obj* object) {
}

void* reallocate(void* pointer, sz_t oldSize, sz_t newSize) {
  vm.bytesAllocated += newSize - oldSize;
  if (newSize > oldSize) {
#if defined(LOXC_DEBUG_STRESS_GC)
    collectGarbage();
#endif

    if (vm.bytesAllocated > vm.nextGC)
      collectGarbage();
  }

  if (newSize <= 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newSize);
  if (result == NULL)
    exit(-1);

  return result;
}

void markObject(Obj* object) {
  if (object == NULL)
    return;
  if (object->isMarked)
    return;

#if defined(LOXC_DEBUG_LOG_GC)
  fprintf(stdout, "%p mark ", (void*)object);
  printValue(OBJ_VAL(object));
  fprintf(stdout, "\n");
#endif

  object->isMarked = true;

  if (vm.grayCapacity < vm.grayCount + 1) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);

    if (vm.grayStack == NULL)
      exit(-1);
  }
  vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value) {
  if (IS_OBJ(value))
    markObject(AS_OBJ(value));
}

void collectGarbage() {}
void freeObjects() {}
