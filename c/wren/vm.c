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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "wren.h"
#include "common.h"
#include "compiler.h"
#include "core.h"
#include "debug.h"
#include "vm.h"

#if WREN_OPT_META
#	include "opt_meta.h"
#endif
#if WREN_OPT_RANDOM
#	include "opt_random.h"
#endif

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
#	include <time.h>
#	include <stdio.h>
#endif

static void* defaultReallocate(void* ptr, sz_t newSize) {
	if (0 == newSize) {
		free(ptr);
		return NULL;
	}
	return realloc(ptr, newSize);
}

static ObjUpvalue* captureUpvalue(WrenVM* vm, ObjFiber* fiber, Value* local) {
	if (NULL == fiber->openUpvalues) {
		fiber->openUpvalues = wrenNewUpvalue(vm, local);
		return fiber->openUpvalues;
	}

	ObjUpvalue* prevUpvalue = NULL;
	ObjUpvalue* upvalue = fiber->openUpvalues;

	while (NULL != upvalue && upvalue->value > local) {
		prevUpvalue = upvalue;
		upvalue = upvalue->next;
	}
	if (NULL != upvalue && upvalue->value == local)
		return upvalue;

	ObjUpvalue* createdUpvalue = wrenNewUpvalue(vm, local);
	if (NULL == prevUpvalue)
		fiber->openUpvalues = createdUpvalue;
	else
		prevUpvalue->next = createdUpvalue;
	createdUpvalue->next = upvalue;

	return createdUpvalue;
}

static void closeUpvalues(ObjFiber* fiber, Value* last) {
	while (NULL != fiber->openUpvalues && fiber->openUpvalues->value >= last) {
		ObjUpvalue* upvalue = fiber->openUpvalues;
		upvalue->closed = *upvalue->value;
		upvalue->value = &upvalue->closed;

		fiber->openUpvalues = upvalue->next;
	}
}

static WrenForeignMethodFn findForeignMethod(WrenVM* vm,
		const char* moduleName, const char* className, bool isStatic, const char* signature) {
	WrenForeignMethodFn method = NULL;
	if (NULL != vm->config.bindForeignMethodFn)
		method = vm->config.bindForeignMethodFn(vm, moduleName, className, isStatic, signature);

	if (NULL == method) {
#if WREN_OPT_META
		if (0 == strcmp(moduleName, "meta"))
			method = wrenMetaBindForeignMethod(vm, className, isStatic, signature);
#endif
#if WREN_OPT_RANDOM
		if (0 == strcmp(moduleName, "random"))
			method = wrenRandomBindForeignMethod(vm, className, isStatic, signature);
#endif
	}

	return method;
}

static void bindMethod(WrenVM* vm,
		int methodType, int symbol, ObjModule* module, ObjClass* classObj, Value methodValue) {
	const char* className = classObj->name->value;
	if (CODE_METHOD_STATIC == methodType)
		classObj = classObj->obj.classObj;

	Method method;
	if (IS_STRING(methodValue)) {
		const char* name = AS_CSTRING(methodValue);
		method.type = METHOD_FOREIGN;
		method.as.foreign = findForeignMethod(vm,
				module->name->value, className, CODE_METHOD_STATIC == methodType, name);

		if (NULL == method.as.foreign) {
			vm->fiber->error = wrenStringFormat(vm,
					"Could not find foreign method `@` for class $ in module `$`.",
					methodValue, classObj->name->value, module->name->value);
			return;
		}
	}
	else {
		method.type = METHOD_BLOCK;
		method.as.closure = AS_CLOSURE(methodValue);
		wrenBindMethodCode(classObj, method.as.closure->fn);
	}

	wrenBindMethod(vm, classObj, symbol, method);
}

static void callForeign(WrenVM* vm, ObjFiber* fiber, WrenForeignMethodFn foreign, int numArgs) {
	ASSERT(NULL == vm->apiStack, "Cannot already be in foreign call.");
	vm->apiStack = fiber->stackTop - numArgs;

	foreign(vm);

	fiber->stackTop = vm->apiStack + 1;
	vm->apiStack = NULL;
}

static void runtimeError(WrenVM* vm) {
	ASSERT(wrenHasError(vm->fiber), "Should only call this after an error.");

	ObjFiber* current = vm->fiber;
	Value error = current->error;

	if (NULL != current) {
		current->error = error;

		if (FIBER_TRY == current->state) {
			current->caller->stackTop[-1] = vm->fiber->error;
			vm->fiber = current->caller;
			return;
		}

		ObjFiber* caller = current->caller;
		current->caller = NULL;
		current = caller;
	}

	wrenDebugPrintStackTrace(vm);
	vm->fiber = NULL;
	vm->apiStack = NULL;
}

static void methodNotFound(WrenVM* vm, ObjClass* classObj, int symbol) {
	vm->fiber->error = wrenStringFormat(vm, "@ does not implement `$`.",
			OBJ_VAL(classObj->name), vm->methodNames.data[symbol]->value);
}

static ObjModule* getModule(WrenVM* vm, Value name) {
	Value moduleValue = wrenMapGet(vm->modules, name);
	return !IS_UNDEFINED(moduleValue) ? AS_MODULE(moduleValue) : NULL;
}

void wrenInitConfiguration(WrenConfiguration* config) {
	config->reallocateFn        = defaultReallocate;
	config->resolveModuleFn     = NULL;
	config->loadModuleFn        = NULL;
	config->bindForeignMethodFn = NULL;
	config->bindForeignClassFn  = NULL;
	config->writeFn             = NULL;
	config->errorFn             = NULL;
	config->initialHeapSize     = 1024 * 1024 * 10;
	config->minHeapSize         = 1024 * 1024;
	config->heapGrowthPercent   = 50;
	config->userData            = NULL;
}

WrenVM* wrenNewVM(WrenConfiguration* config) {
	WrenReallocateFn reallocate = defaultReallocate;
	if (NULL != config)
		reallocate = config->reallocateFn;

	WrenVM* vm = (WrenVM*)reallocate(NULL, sizeof(*vm));
	memset(vm, 0, sizeof(WrenVM));

	if (NULL != config)
		memcpy(&vm->config, config, sizeof(WrenConfiguration));
	else
		wrenInitConfiguration(&vm->config);

	vm->grayCount = 0;
	vm->grayCapacity = 4;
	vm->gray = (Obj**)reallocate(NULL, vm->grayCapacity * sizeof(Obj*));
	vm->nextGC = vm->config.initialHeapSize;

	wrenSymbolTableInit(&vm->methodNames);

	vm->modules = wrenNewMap(vm);
	wrenInitializeCore(vm);

	return vm;
}

void wrenFreeVM(WrenVM* vm) {
	ASSERT(vm->methodNames.count > 0, "VM appears to have a already been freed.");

	Obj* obj = vm->first;
	while (NULL != obj) {
		Obj* next = obj->next;
		wrenFreeObj(vm, obj);
		obj = next;
	}

	vm->gray = (Obj**)vm->config.reallocateFn(vm->gray, 0);
	ASSERT(NULL == vm->handles, "All handles have not been released.");

	wrenSymbolTableClear(vm, &vm->methodNames);
	DEALLOCATE(vm, vm);
}

void wrenCollectGrabage(WrenVM* vm) {
#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
	fprintf(stdout, "--------- gc ---------\n");
	sz_t before = vm->bytesAllocated;
	double startTime = (double)clock() / CLOCKS_PER_SEC;
#endif

	vm->bytesAllocated = 0;

	wrenGrayObj(vm, (Obj*)vm->modules);
	for (int i = 0; i < vm->numTempRoots; ++i)
		wrenGrayObj(vm, vm->tempRoots[i]);
	wrenGrayObj(vm, (Obj*)vm->fiber);
	for (WrenHandle* handle = vm->handles; NULL != handle; handle = handle->next)
		wrenGrayValue(vm, handle->value);
	if (NULL != vm->compiler)
		wrenMarkCompiler(vm, vm->compiler);

	wrenBlackenSymbolTable(vm, &vm->methodNames);
	wrenBlackenObjects(vm);

	Obj** obj = &vm->first;
	while (NULL != *obj) {
		if (!((*obj)->isDark)) {
			Obj* unreached = *obj;
			*obj = unreached->next;
			wrenFreeObj(vm, unreached);
		}
		else {
			(*obj)->isDark = false;
			obj = &(*obj)->next;
		}
	}

	vm->nextGC = vm->bytesAllocated + ((vm->bytesAllocated * vm->config.heapGrowthPercent) / 100);
	if (vm->nextGC < vm->config.minHeapSize)
		vm->nextGC = vm->config.minHeapSize;

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
	double elapsed = ((double)clock() / CLOCKS_PER_SEC) - startTime;
	fprintf(stdout, "GC %lu before, %lu after (%lu collected), next at %lu. Took %.3fs.\n",
			(unsigned long)before, (unsigned long)vm->bytesAllocated,
			(unsigned long)(before - vm->bytesAllocated), (unsigned long)vm->nextGC, elapsed);
#endif
}

void* wrenReallocate(WrenVM* vm, void* memory, sz_t oldSize, sz_t newSize) {
#if WREN_DEBUG_TRACE_MEMORY
	fprintf(stdout, "reallocate %p %lu -> %lu\n", memory, (unsigned long)oldSize, (unsigned long)newSize);
#endif

	vm->bytesAllocated += newSize - oldSize;

#if WREN_DEBUG_GC_STRESS
	if (newSize > 0)
		wrenCollectGrabage(vm);
#else
	if (newSize > 0 && vm->bytesAllocated > vm->nextGC)
		wrenCollectGrabage(vm);
#endif

	return vm->config.reallocateFn(memory, newSize);
}

void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign) {}
WrenHandle* wrenMakeHandle(WrenVM* vm, Value value) { return NULL; }
ObjClosure* wrenCompileSource(WrenVM* vm,
		const char* module, const char* sourceCode, bool isExpression, bool printErrors) { return NULL; }
Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName) {
	Value v; v.type = VAL_UNDEFINED; return v;
}
Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name) {
	Value v; v.type = VAL_UNDEFINED; return v;
}
int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name, sz_t length, int lineno) { return 0; }
int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name, sz_t length, Value value) { return 0; }
void wrenPushRoot(WrenVM* vm, Obj* obj) {}
void wrenPopRoot(WrenVM* vm) {}
