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
#ifndef WREN_VM_H
#define WREN_VM_H

#include "common.h"
#include "utils.h"
#include "value.h"
#include "compiler.h"

#define WREN_MAX_TEMP_ROOTS                 (5)

typedef enum {
#undef WREN_OPCODE
#define WREN_OPCODE(name, _) CODE_##name,
#include "opcodes.h"
#undef WREN_OPCODE
} Code;

struct WrenHandle {
	Value               value;

	WrenHandle*         prev;
	WrenHandle*         next;
};

struct WrenVM {
	ObjClass*           boolClass;
	ObjClass*           classClass;
	ObjClass*           fiberClass;
	ObjClass*           fnClass;
	ObjClass*           listClass;
	ObjClass*           mapClass;
	ObjClass*           nullClass;
	ObjClass*           numClass;
	ObjClass*           objectClass;
	ObjClass*           rangeClass;
	ObjClass*           stringClass;

	ObjFiber*           fiber;
	ObjMap*             modules;
	ObjModule*          lastModule;

	sz_t                bytesAllocated;
	sz_t                nextGC;

	Obj*                first;
	Obj**               gray;
	int                 grayCount;
	int                 grayCapacity;

	Obj*                tempRoots[WREN_MAX_TEMP_ROOTS];
	int                 numTempRoots;

	WrenHandle*         handles;
	Value*              apiStack;

	WrenConfiguration   config;
	Compiler*           compiler;
	SymbolTable         methodNames;
};

void* wrenReallocate(WrenVM* vm, void* memory, sz_t oldSize, sz_t newSize);
void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign);
WrenHandle* wrenMakeHandle(WrenVM* vm, Value value);

ObjClosure* wrenCompileSource(WrenVM* vm,
		const char* module, const char* sourceCode, bool isExpression, bool printErrors);

Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName);
Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name);
int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name, sz_t length, int lineno);
int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name, sz_t length, Value value);

static inline void wrenCallFunction(WrenVM* vm, ObjFiber* fiber, ObjClosure* closure, int numArgs) {
	if (fiber->numFrames + 1 > fiber->frameCapacity) {
		int maxFrameCapacity = fiber->frameCapacity * 2;
		fiber->frames = (CallFrame*)wrenReallocate(vm, fiber->frames,
				sizeof(CallFrame) * fiber->frameCapacity, sizeof(CallFrame) * maxFrameCapacity);
		fiber->frameCapacity = maxFrameCapacity;
	}

	int stackSize = (int)(fiber->stackTop - fiber->stack);
	int needed = stackSize + closure->fn->maxSlots;
	wrenEnsureStack(vm, fiber, needed);

	wrenAppendCallFrame(vm, fiber, closure, fiber->stackTop - numArgs);
}

void wrenPushRoot(WrenVM* vm, Obj* obj);
void wrenPopRoot(WrenVM* vm);

static inline ObjClass* wrenGetClassInline(WrenVM* vm, Value value) {
	if (IS_NUM(value))
		return vm->numClass;
	if (IS_OBJ(value))
		return AS_OBJ(value)->classObj;

	switch (value.type) {
	case VAL_FALSE:         return vm->boolClass;
	case VAL_NULL:          return vm->nullClass;
	case VAL_NUM:           return vm->numClass;
	case VAL_TRUE:          return vm->boolClass;
	case VAL_OBJ:           return AS_OBJ(value)->classObj;
	case VAL_UNDEFINED:     UNREACHABLE(); break;
	default:                UNREACHABLE(); break;
	}
	return NULL;
}

#endif
