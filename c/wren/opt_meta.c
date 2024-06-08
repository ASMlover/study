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
#include "opt_meta.h"
#include "vm.h"
#include "opt_meta.wren.inc"

static void metaCompile(WrenVM* vm) {
	const char* sourceCode = wrenGetSlotString(vm, 1);
	bool isExpression = wrenGetSlotBool(vm, 2);
	bool printErrors = wrenGetSlotBool(vm, 3);

	ObjFiber* currentFiber = vm->fiber;
	ObjFn* fn = currentFiber->frames[currentFiber->numFrames - 2].closure->fn;
	ObjString* moduleName = fn->module->name;

	ObjClosure* closure = wrenCompileSource(vm, moduleName->value, sourceCode, isExpression, printErrors);

	if (NULL == closure)
		vm->apiStack[0] = NULL_VAL;
	else
		vm->apiStack[0] = OBJ_VAL(closure);
}

static void metaGetModuleVariables(WrenVM* vm) {
	wrenEnsureSlots(vm, 3);

	Value moduleValue = wrenMapGet(vm->modules, vm->apiStack[1]);
	if (IS_UNDEFINED(moduleValue)) {
		vm->apiStack[0] = NULL_VAL;
		return;
	}

	ObjModule* module = AS_MODULE(moduleValue);
	ObjList* names = wrenNewList(vm, module->variableNames.count);
	vm->apiStack[0] = OBJ_VAL(names);

	for (int i = 0; i < names->elements.count; ++i)
		names->elements.data[i] = NULL_VAL;
	for (int i = 0; i < names->elements.count; ++i)
		names->elements.data[i] = OBJ_VAL(module->variableNames.data[i]);
}

const char* wrenMetaSource() {
	return metaModuleSource;
}

WrenForeignMethodFn wrenMetaBindForeignMethod(WrenVM* vm,
		const char* className, bool isStatic, const char* signature) {
	ASSERT(0 == strcmp(className, "Meta"), "Should be in Meta class.");
	ASSERT(isStatic, "Should be static.");

	if (0 == strcmp(signature, "compile_(_,_,_)"))
		return metaCompile;
	else if (0 == strcmp(signature, "metaGetModuleVariables_(_)"))
		return metaGetModuleVariables;

	ASSERT(false, "Unknown method.");
	return NULL;
}
