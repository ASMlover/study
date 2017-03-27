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
#include <stdio.h>
#include <stdlib.h>
#include "ms.h"

#define MAX_STACK         (256)
#define INIT_GC_THRESHOLD (8)

typedef enum _marktype {
  UNMARKED,
  MARKED,
} MarkType;

struct _vm {
  Object* stack[MAX_STACK];
  int nstack;

  Object* head_obj;
  int nobj;
  int max_nobj;
};

#define CHECK(cond, msg) do {\
  if (!(cond)) {\
    fprintf(stderr, "%s\n", msg);\
    abort();\
  }\
} while (0)

static void vm_push(VM* vm, Object* obj) {
  CHECK(vm->nstack < MAX_STACK, "stack overflow ...");
  vm->stack[vm->nstack++] = obj;
}

static Object* vm_pop(VM* vm) {
  CHECK(vm->nstack > 0, "stack underflow ...");
  return vm->stack[--vm->nstack];
}

static void gc_mark(Object* obj) {
  if (obj->marked == MARKED)
    return;

  obj->marked = MARKED;
  if (obj->type == OBJECT_PAIR) {
    gc_mark(obj->first);
    gc_mark(obj->second);
  }
}

static void gc_mark_all(VM* vm) {
  for (int i = 0; i < vm->nstack; ++i)
    gc_mark(vm->stack[i]);
}

static void gc_sweep(VM* vm) {
  Object** obj = &vm->head_obj;
  while (*obj) {
    if ((*obj)->marked == UNMARKED) {
      Object* unmarked = *obj;

      *obj = unmarked->next;
      free(unmarked);

      --vm->nobj;
    }
    else {
      (*obj)->marked = UNMARKED;
      obj = &(*obj)->next;
    }
  }
}

Object* new_object(VM* vm, ObjectType type) {
  if (vm->nobj >= vm->max_nobj)
    gc_collect(vm);

  Object* obj = (Object*)malloc(sizeof(Object));
  obj->type = type;
  obj->marked = UNMARKED;
  obj->next = vm->head_obj;
  vm->head_obj = obj;

  ++vm->nobj;

  return obj;
}

VM* new_vm(void) {
  VM* vm = (VM*)malloc(sizeof(VM));
  CHECK(vm != NULL, "create VM should success ...");

  vm->nstack = 0;
  vm->head_obj = NULL;
  vm->nobj = 0;
  vm->max_nobj = INIT_GC_THRESHOLD;

  return vm;
}

void free_vm(VM* vm) {
  vm->nstack = 0;
  gc_collect(vm);
  free(vm);
}

Object* push_int(VM* vm, int value) {
  Object* obj = new_object(vm, OBJECT_INT);
  obj->value = value;

  vm_push(vm, obj);
  return obj;
}

Object* push_pair(VM* vm) {
  Object* obj = new_object(vm, OBJECT_PAIR);
  obj->second = vm_pop(vm);
  obj->first = vm_pop(vm);

  vm_push(vm, obj);
  return obj;
}

Object* pop_object(VM* vm) {
  return vm_pop(vm);
}

void gc_collect(VM* vm) {
  int nobj = vm->nobj;

  gc_mark_all(vm);
  gc_sweep(vm);

  vm->max_nobj = (int)(vm->max_nobj * 1.5);

  fprintf(stdout, "collected `%d` objects, `%d` remaining.\n", nobj - vm->nobj, vm->nobj);
}
