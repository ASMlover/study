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
#ifndef WREN_H
#define WREN_H

#include "common.h"

typedef struct WrenVM WrenVM;
typedef struct WrenHandle WrenHandle;

typedef void* (*WrenReallocateFn)(void* memory, sz_t newSize);
typedef void  (*WrenForeignMethodFn)(WrenVM* vm);
typedef void  (*WrenFinalizerFn)(void* data);

typedef const char* (*WrenResolveModuleFn)(WrenVM* vm, const char* importer, const char* name);
typedef char* (*WrenLoadModuleFn)(WrenVM* vm, const char* name);
typedef WrenForeignMethodFn (*WrenBindForeignMethodFn)(WrenVM* vm,
		const char* module, const char* className, bool isStatic, const char* signature);
typedef void (*WrenWriteFn)(WrenVM* vm, const char* text);

typedef enum {
	WREN_ERROR_COMPILE,
	WREN_ERROR_RUNTIME,
	WREN_ERROR_STACK_TRACE,
} WrenErrorType;

typedef void (*WrenErrorFn)(WrenVM* vm,
		WrenErrorType type, const char* module, int lineno, const char* message);

typedef struct {
	WrenForeignMethodFn allocate;
	WrenFinalizerFn finalize;
} WrenForeignClassMethods;
typedef WrenForeignClassMethods (*WrenBindForeignClassFn)(WrenVM* vm,
		const char* module, const char* className);

typedef struct {
	WrenReallocateFn        reallocateFn;
	WrenResolveModuleFn     resolveModuleFn;
	WrenLoadModuleFn        loadModuleFn;
	WrenBindForeignMethodFn bindForeignMethodFn;
	WrenBindForeignClassFn  bindForeignClassFn;
	WrenWriteFn             writeFn;
	WrenErrorFn             errorFn;
	sz_t                    initialHeapSize;
	sz_t                    minHeapSize;
	int                     heapGrowthPercent;
	void*                   userData;
} WrenConfiguration;

typedef enum {
	WREN_RESULT_SUCCESS,
	WREN_RESULT_COMPILE_ERROR,
	WREN_RESULT_RUNTIME_ERROR,
} WrenInterpretResult;

typedef enum {
	WREN_TYPE_BOOL,
	WREN_TYPE_NUM,
	WREN_TYPE_FOREIGN,
	WREN_TYPE_LIST,
	WREN_TYPE_NULL,
	WREN_TYPE_STRING,

	WREN_TYPE_UNKNOWN,
} WrenType;

void wrenInitConfiguration(WrenConfiguration* configuration);

WrenVM* wrenNewVM(WrenConfiguration* configuration);
void wrenFreeVM(WrenVM* vm);

void wrenCollectGrabage(WrenVM* vm);
WrenInterpretResult wrenInterpret(WrenVM* vm, const char* module, const char* sourceCode);

WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature);
WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method);
void wrenReleaseHandle(WrenVM* vm, WrenHandle* handle);

int wrenGetSlotCount(WrenVM* vm);
void wrenEnsureSlots(WrenVM* vm, int numSlots);
WrenType wrenGetSlotType(WrenVM* vm, int slot);

bool wrenGetSlotBool(WrenVM* vm, int slot);
const char* wrenGetSlotBytes(WrenVM* vm, int slot, int* length);
double wrenGetSlotDouble(WrenVM* vm, int slot);
void* wrenGetSlotForeign(WrenVM* vm, int slot);
const char* wrenGetSlotString(WrenVM* vm, int slot);
WrenHandle* wrenGetSlotHandle(WrenVM* vm, int slot);

void wrenSetSlotBool(WrenVM* vm, int slot, bool value);
void wrenSetSlotBytes(WrenVM* vm, int slot, const char* bytes, sz_t length);
void wrenSetSlotDouble(WrenVM* vm, int slot, double value);
void* wrenSetSlotNewForeign(WrenVM* vm, int slot, int classSlot, sz_t size);
void wrenSetSlotNewList(WrenVM* vm, int slot);
void wrenSetSlotNull(WrenVM* vm, int slot);
void wrenSetSlotString(WrenVM* vm, int slot, const char* text);
void wrenSetSlotHandle(WrenVM* vm, int slot, WrenHandle* handle);

int wrenGetListCount(WrenVM* vm, int slot);
void wrenGetListElement(WrenVM* vm, int listSlot, int index, int elementSlot);
void wrenInsertInList(WrenVM* vm, int listSlot, int index, int elementSlot);

void wrenGetVariable(WrenVM* vm, const char* module, const char* name, int slot);
void wrenAbortFiber(WrenVM* vm, int slot);

void* wrenGetUserData(WrenVM* vm);
void wrenSetUserData(WrenVM* vm, void* userData);

#endif
