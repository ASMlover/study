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
#include "compiler.h"
#include "vm.h"

#define GC_HEAP_GROW_FACTOR (2)

static void markArray(ValueArray* array) {
	for (int i = 0; i < array->count; ++i)
		markValue(array->values[i]);
}

static void blackenObject(Obj* object) {
#if defined(DEBUG_LOG_GC)
	fprintf(stdout, "%p blacken ", (void*)object);
	printValue(OBJ_VAL(object));
	fprintf(stdout, "\n");
#endif

	switch (object->type) {
	case OBJ_BOUND_METHOD:
		{
			ObjBoundMethod* bound = (ObjBoundMethod*)object;
			markValue(bound->receiver);
			markObject((Obj*)bound->method);
		} break;
	case OBJ_CLASS:
		{
			ObjClass* klass = (ObjClass*)object;
			markObject((Obj*)klass->name);
			markTable(&klass->methods);
		} break;
	case OBJ_CLOSURE:
		{
			ObjClosure* closure = (ObjClosure*)object;
			markObject((Obj*)closure->function);
			for (int i = 0; i < closure->upvalueCount; ++i)
				markObject((Obj*)closure->upvalues[i]);
		} break;
	case OBJ_FUNCTION:
		{
			ObjFunction* function = (ObjFunction*)object;
			markObject((Obj*)function->name);
			markArray(&function->chunk.constants);
		} break;
	case OBJ_INSTANCE:
		{
			ObjInstance* instance = (ObjInstance*)object;
			markObject((Obj*)instance->klass);
			markTable(&instance->fields);
		} break;
	case OBJ_UPVALUE: markValue(((ObjUpvalue*)object)->closed); break;
	case OBJ_NATIVE:
	case OBJ_STRING: break;
	default: break;
	}
}

static void freeObject(Obj* object) {
#if defined(CLOX_DEBUG_LOG_GC)
	fprintf(stdout, "%p free type %d\n", (void*)object, object->type);
#endif

	switch (object->type) {
	case OBJ_BOUND_METHOD: FREE(ObjBoundMethod, object); break;
	case OBJ_CLASS:
		{
			ObjClass* klass = (ObjClass*)object;
			freeTable(&klass->methods);
			FREE(ObjClass, object);
		} break;
	case OBJ_CLOSURE:
		{
			ObjClosure* closure = (ObjClosure*)object;
			FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
			FREE(ObjClosure, object);
		} break;
	case OBJ_FUNCTION:
		{
			ObjFunction* function = (ObjFunction*)object;
			freeChunk(&function->chunk);
			FREE(ObjFunction, object);
		} break;
	case OBJ_INSTANCE:
		{
			ObjInstance* instance = (ObjInstance*)object;
			freeTable(&instance->fields);
			FREE(ObjInstance, object);
		} break;
	case OBJ_NATIVE: FREE(ObjNative, object); break;
	case OBJ_STRING:
		{
			ObjString* string = (ObjString*)object;
			FREE_ARRAY(char, string->chars, string->length + 1);
			FREE(ObjString, object);
		} break;
	case OBJ_UPVALUE: FREE(ObjUpvalue, object); break;
	default: break;
	}
}

static void traceReferences() {
	while (vm.grayCount > 0) {
		Obj* object = vm.grayStack[--vm.grayCount];
		blackenObject(object);
	}
}

static void markRoots() {
	for (Value* slot = vm.stack; slot < vm.stackTop; ++slot)
		markValue(*slot);

	for (int i = 0; i < vm.frameCount; ++i)
		markObject((Obj*)vm.frames[i].closure);

	for (ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next)
		markObject((Obj*)upvalue);

	markTable(&vm.globals);
	markCompilerRoots();
	markObject((Obj*)vm.initString);
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

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	vm.bytesAllocated += newSize - oldSize;
	if (newSize > oldSize) {
#if defined(DEBUG_STRESS_GC)
		collectGarbage();
#endif

		if (vm.bytesAllocated >= vm.nextGC)
			collectGarbage();
	}

	if (pointer != NULL && newSize <= 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, newSize);
	if (result == NULL)
		exit(1);
	return result;
}

void markObject(Obj* object) {
	if (object == NULL || object->isMarked)
		return;

#if defined(DEBUG_LOG_GC)
	fprintf(stdout, "%p mark ", (void*)object);
	printValue(OBJ_VAL(object));
	fprintf(stdout, "\n");
#endif

	object->isMarked = true;
	if (vm.grayCapacity < vm.grayCount + 1) {
		vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
		vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);

		if (vm.grayStack == NULL)
			exit(1);
	}
	vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value) {
	if (IS_OBJ(value))
		markObject(AS_OBJ(value));
}

void collectGarbage() {
#if defined(CLOX_DEBUG_LOG_GC)
	fprintf(stdout, "--------- gc begin ---------\n");
	size_t before = vm.bytesAllocated;
#endif

	markRoots();
	traceReferences();

	tableRemoveWhite(&vm.strings);
	sweep();

	vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#if defined(CLOX_DEBUG_LOG_GC)
	fprintf(stdout, "--------- gc end ---------\n");
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
