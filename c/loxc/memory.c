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
#include "memory.h"
#include "value.h"
#include "compiler.h"
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

static inline void freeClass(Obj* object) {
  ObjClass* klass = (ObjClass*)object;
  freeTable(&klass->methods);
  FREE(ObjClass, object);
}

static inline void freeClosure(Obj* object) {
  ObjClosure* closure = (ObjClosure*)object;
  FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
  FREE(ObjClosure, object);
}

static inline void freeFunction(Obj* object) {
  ObjFunction* function = (ObjFunction*)object;
  freeChunk(&function->chunk);
  FREE(ObjFunction, object);
}

static inline void freeInstance(Obj* object) {
  ObjInstance* instance = (ObjInstance*)object;
  freeTable(&instance->fields);
  FREE(ObjInstance, object);
}

static inline void freeNative(Obj* object) {
  FREE(ObjNative, object);
}

static inline void freeString(Obj* object) {
  ObjString* string = (ObjString*)object;
  FREE_ARRAY(char, string->chars, string->length + 1);
  FREE(ObjString, object);
}

static inline void freeUpvalue(Obj* object) {
  FREE(ObjUpvalue, object);
}

static void freeObject(Obj* object) {
#if defined(LOXC_DEBUG_LOG_GC)
  fprintf(stdout, "%p free type %d\n", (void*)object, object->type);
#endif

  switch (object->type) {
  case OBJ_BOUND_METHOD:  freeBoundMethod(object); break;
  case OBJ_CLASS:         freeClass(object); break;
  case OBJ_CLOSURE:       freeClosure(object); break;
  case OBJ_FUNCTION:      freeFunction(object); break;
  case OBJ_INSTANCE:      freeInstance(object); break;
  case OBJ_NATIVE:        freeNative(object); break;
  case OBJ_STRING:        freeString(object); break;
  case OBJ_UPVALUE:       freeUpvalue(object); break;
  default: break;
  }
}

static void markRoots() {
  for (Value* slot = vm.stack; slot < vm.stackTop; ++slot)
    MARK_VAL(*slot);

  for (int i = 0; i < vm.frameCount; ++i)
    MARK_OBJ(vm.frames[i].closure);

  for (ObjUpvalue* upvalue = vm.openValues; upvalue != NULL; upvalue = upvalue->next)
    MARK_OBJ(upvalue);

  markTable(&vm.globals);
  markCompilerRoots();
  MARK_OBJ(vm.initString);
}

static void traceReferences() {
  while (vm.grayCount > 0) {
    Obj* object = vm.grayStack[--vm.grayCount] ;
    blackenObject(object);
  }
}

static void sweep() {
  Obj* previous = NULL;
  Obj* object = vm.objects;
  while (object != NULL) {
    if (object->isMarked) {
      object->isMarked = false;
      previous = object;
      object = object->next;
    }
    else {
      Obj* unreached = object;
      object = object->next;
      if (previous != NULL)
        previous->next = object;
      else
        vm.objects = object;

      freeObject(unreached);
    }
  }
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

void collectGarbage() {
#if defined(LOXC_DEBUG_LOG_GC)
  fprintf(stdout, "--------- gc begin\n");
  sz_t before = vm.bytesAllocated;
#endif

  markRoots();
  traceReferences();
  tableRemoveWhite(&vm.strings);
  sweep();

  vm.nextGC = vm.bytesAllocated * LOXC_GC_HEAP_GROW_FACTOR;

#if defined(LOXC_DEBUG_LOG_GC)
  fprintf(stdout, "--------- gc end\n");
  fprintf(stdout, "  collected %zu bytes (from %zu to %zu) next at %zu\n",
      before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

void freeObjects() {
  Obj* object = vm.objects;
  while (object != NULL) {
    Obj* next = object->next;
    freeObject(object);
    object = next;
  }

  free(vm.grayStack);
}
