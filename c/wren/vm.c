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

static ObjModule* getModule(WrenVM* vm, Value moduleName) {
	Value moduleValue = wrenMapGet(vm->modules, moduleName);
	return !IS_UNDEFINED(moduleValue) ? AS_MODULE(moduleValue) : NULL;
}

static ObjClosure* compileInModule(WrenVM* vm,
		Value moduleName, const char* sourceCode, bool isExpression, bool printErrors) {
	ObjModule* module = getModule(vm, moduleName);
	if (NULL == module) {
		module = wrenNewModule(vm, AS_STRING(moduleName));
		wrenMapSet(vm, vm->modules, moduleName, OBJ_VAL(module));

		ObjModule* coreModule = getModule(vm, NULL_VAL);
		for (int i = 0; i < coreModule->variables.count; ++i) {
			wrenDefineVariable(vm, module,
					coreModule->variableNames.data[i]->value,
					coreModule->variableNames.data[i]->length,
					coreModule->variables.data[i]);
		}
	}

	ObjFn* fn = wrenCompile(vm, module, sourceCode, isExpression, printErrors);
	if (NULL == fn)
		return NULL;

	wrenPushRoot(vm, (Obj*)fn);
	ObjClosure* closure = wrenNewClosure(vm, fn);
	wrenPopRoot(vm);

	return closure;
}

static Value validateSuperclass(WrenVM* vm, Value name, Value superclassValue, int numFields) {
	if (!IS_CLASS(superclassValue))
		return wrenStringFormat(vm, "Class `@` cannot inherit from a non-class object.", name);

	ObjClass* superclass = AS_CLASS(superclassValue);
	if (superclass == vm->classClass ||
			superclass == vm->fiberClass ||
			superclass == vm->fnClass ||
			superclass == vm->listClass ||
			superclass == vm->mapClass ||
			superclass == vm->rangeClass ||
			superclass == vm->stringClass) {
		return wrenStringFormat(vm, "Class `@` cannot inherit from built-in class `@`.",
				name, OBJ_VAL(superclass->name));
	}

	if (-1 == superclass->numFields) {
		return wrenStringFormat(vm, "Class `@` cannot inherit from foreign class `@`.",
				name, OBJ_VAL(superclass->name));
	}
	if (-1 == numFields && superclass->numFields > 0)
		return wrenStringFormat(vm, "Foreign class `@` may not inherit from a class with fields.", name);
	if (superclass->numFields + numFields > WREN_MAX_FIELDS) {
		return wrenStringFormat(vm,
				"Class `@` may not have more than 255 fields, including inherited ones.", name);
	}

	return NULL_VAL;
}

static void bindForeignClass(WrenVM* vm, ObjClass* classObj, ObjModule* module) {
	WrenForeignClassMethods methods;
	methods.allocate = NULL;
	methods.finalize = NULL;

	if (NULL != vm->config.bindForeignClassFn)
		methods = vm->config.bindForeignClassFn(vm, module->name->value, classObj->name->value);

	if (NULL == methods.allocate && NULL == methods.finalize) {
#if WREN_OPT_RANDOM
		if (0 == strcmp(module->name->value, "random"))
			methods = wrenRandomBindForeignClass(vm, module->name->value, classObj->name->value);
#endif
	}

	Method method;
	method.type = METHOD_FOREIGN;

	int symbol = wrenSymbolTableEnsure(vm, &vm->methodNames, "<allocate>", 10);
	if (NULL != methods.allocate) {
		method.as.foreign = (WrenForeignMethodFn)methods.finalize;
		wrenBindMethod(vm, classObj, symbol, method);
	}
}

static void createClass(WrenVM* vm, int numFields, ObjModule* module) {
	Value name = vm->fiber->stackTop[-2];
	Value superclass = vm->fiber->stackTop[-1];

	--vm->fiber->stackTop;
	vm->fiber->error = validateSuperclass(vm, name, superclass, numFields);

	ObjClass* classObj = wrenNewClass(vm, AS_CLASS(superclass), numFields, AS_STRING(name));
	vm->fiber->stackTop[-1] = OBJ_VAL(classObj);

	if (-1 == numFields)
		bindForeignClass(vm, classObj, module);
}

static void createForeign(WrenVM* vm, ObjFiber* fiber, Value* stack) {
	ObjClass* classObj = AS_CLASS(stack[0]);
	ASSERT(-1 == classObj->numFields, "Class must be a foreign class.");

	int symbol = wrenSymbolTableFind(&vm->methodNames, "<allocate>", 10);
	ASSERT(-1 != symbol, "Should have defined <allocate> symbol.");

	ASSERT(classObj->methods.count > symbol, "Class should have allocator.");
	Method* method = &classObj->methods.data[symbol];
	ASSERT(METHOD_FOREIGN == method->type, "Allocator should be foreign.");

	ASSERT(NULL == vm->apiStack, "Cannot already be in foreign call.");
	vm->apiStack = stack;

	method->as.foreign(vm);

	vm->apiStack = NULL;
}

static Value resolveModule(WrenVM* vm, Value name) {
	if (NULL == vm->config.resolveModuleFn)
		return name;

	ObjFiber* fiber = vm->fiber;
	ObjFn* fn = fiber->frames[fiber->numFrames - 1].closure->fn;
	ObjString* importer = fn->module->name;

	const char* resolved = vm->config.resolveModuleFn(vm, importer->value, AS_CSTRING(name));
	if (NULL == resolved) {
		vm->fiber->error = wrenStringFormat(vm,
				"Could not resolve module `@` imported from `@`.", name, OBJ_VAL(importer));
		return NULL_VAL;
	}

	if (resolved == AS_CSTRING(name))
		return name;

	name = wrenNewString(vm, resolved);
	DEALLOCATE(vm, (char*)resolved);
	return name;
}

static Value importModule(WrenVM* vm, Value name) {
	name = resolveModule(vm, name);

	Value existing = wrenMapGet(vm->modules, name);
	if (!IS_UNDEFINED(existing))
		return existing;

	wrenPushRoot(vm, AS_OBJ(name));

	const char* sourceCode = NULL;
	bool allocatedSource = true;

	if (NULL != vm->config.loadModuleFn)
		sourceCode = vm->config.loadModuleFn(vm, AS_CSTRING(name));

	if (NULL == sourceCode) {
		ObjString* nameString = AS_STRING(name);

#if WREN_OPT_META
		if (0 == strcmp(nameString->value, "meta"))
			sourceCode = wrenMetaSource();
#endif
#if WREN_OPT_RANDOM
		if (0 == strcmp(nameString->value, "random"))
			sourceCode = wrenRandomSource();
#endif

		allocatedSource = false;
	}

	if (NULL == sourceCode) {
		vm->fiber->error = wrenStringFormat(vm, "Could not load module `@`.", name);
		wrenPopRoot(vm);
		return NULL_VAL;
	}

	ObjClosure* moduleClosure = compileInModule(vm, name, sourceCode, false, true);
	if (allocatedSource)
		DEALLOCATE(vm, (char*)sourceCode);

	if (NULL == moduleClosure) {
		vm->fiber->error = wrenStringFormat(vm, "Could not compile module `@`.", name);
		wrenPopRoot(vm);
		return NULL_VAL;
	}

	wrenPopRoot(vm);
	return OBJ_VAL(moduleClosure);
}

static Value getModuleVariable(WrenVM* vm, ObjModule* module, Value variableName) {
	ObjString* variable = AS_STRING(variableName);
	u32_t variableEntry = wrenSymbolTableFind(&module->variableNames, variable->value, variable->length);

	if (UINT32_MAX != variableEntry)
		return module->variables.data[variableEntry];

	vm->fiber->error = wrenStringFormat(vm,
			"Could not find a variable named `@` in module `@`.", variableName, OBJ_VAL(module->name));
	return NULL_VAL;
}

static WrenInterpretResult runInterpreter(WrenVM* vm, register ObjFiber* fiber) {
	vm->fiber = fiber;
	fiber->state = FIBER_ROOT;

	register CallFrame* frame;
	register Value* stackStart;
	register u8_t* ip;
	register ObjFn* fn;

#define PUSH(value)                         (*fiber->stackTop++ = value)
#define POP()                               (*(--fiber->stackTop))
#define DROP()                              (fiber->stackTop--)
#define PEEK()                              (*(fiber->stackTop - 1))
#define PEEK2()                             (*(fiber->stackTop - 2))
#define READ_BYTE()                         (*ip++)
#define READ_SHORT()                        (ip += 2, (u16_t)((ip[-2] << 8) | ip[-1]))
#define STORE_FRAME()                       frame->ip = ip
#define LOAD_FRAME()\
	frame = &fiber->frames[fiber->numFrames - 1];\
	stackStart = frame->stackStart;\
	ip = frame->ip;\
	fn = frame->closure->fn

#define RUNTIME_ERROR()\
	do {\
		STORE_FRAME();\
		runtimeError(vm);\
		if (NULL == vm->fiber)\
			return WREN_RESULT_RUNTIME_ERROR;\
		fiber = vm->fiber;\
		LOAD_FRAME();\
		DISPATCH();\
	} while (false)

#if WREN_DEBUG_TRACE_INSTRUCTIONS
#	define DEBUG_TRACE_INSTRUCTIONS()\
		do {\
			wrenDumpStack(fiber);\
			wrenDumpInstruction(vm, fn, (int)(ip - fn->code.data));\
		} while (false)
#else
#	define DEBUG_TRACE_INSTRUCTIONS()       do {} while (false)
#endif

#if WREN_COMPUTED_GOTO
	static void* dispatchTable[] = {
#		define OPCODE(name, _) &&code_##name,
#		include "opcodes.h"
#		undef OPCODE
	};
#	define INTERPRET_LOOP                   DISPATCH();
#	define CASE_CODE(name)                  code_##name:

#	define DISPATCH()\
		do {\
			DEBUG_TRACE_INSTRUCTIONS();\
			goto *dispatchTable[instruction = (Code)READ_BYTE()];\
		} while (false)
#else
#	define INTERPRET_LOOP\
		loop:\
			DEBUG_TRACE_INSTRUCTIONS();\
			switch (instruction = (Code)READ_BYTE())
#	define CASE_CODE(name)                  case CODE_##name:
#	define DISPATCH()                       goto loop
#endif

	LOAD_FRAME();
	Code instruction;
	INTERPRET_LOOP {
	CASE_CODE(LOAD_LOCAL_0)
	CASE_CODE(LOAD_LOCAL_1)
	CASE_CODE(LOAD_LOCAL_2)
	CASE_CODE(LOAD_LOCAL_3)
	CASE_CODE(LOAD_LOCAL_4)
	CASE_CODE(LOAD_LOCAL_5)
	CASE_CODE(LOAD_LOCAL_6)
	CASE_CODE(LOAD_LOCAL_7)
	CASE_CODE(LOAD_LOCAL_8) {
		PUSH(stackStart[instruction - CODE_LOAD_LOCAL_0]);
		DISPATCH();
	}

	CASE_CODE(LOAD_LOCAL) {
		PUSH(stackStart[READ_BYTE()]);
		DISPATCH();
	}

	CASE_CODE(LOAD_FIELD_THIS) {
		u8_t field = READ_BYTE();
		Value receiver = stackStart[0];
		ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
		ObjInstance* instance = AS_INSTANCE(receiver);
		ASSERT(field < instance->obj.classObj->numFields, "Out of bounds fields.");
		PUSH(instance->fields[field]);
		DISPATCH();
	}

	CASE_CODE(POP)      DROP(); DISPATCH();
	CASE_CODE(NULL)     PUSH(NULL_VAL); DISPATCH();
	CASE_CODE(FALSE)    PUSH(FALSE_VAL); DISPATCH();
	CASE_CODE(TRUE)     PUSH(TRUE_VAL); DISPATCH();

	CASE_CODE(STORE_LOCAL) {
		PUSH(fn->constants.data[READ_SHORT()]);
		DISPATCH();
	}

	CASE_CODE(CONSTANT) {
		stackStart[READ_BYTE()] = PEEK();
		DISPATCH();
	}

	{
		int numArgs;
		int symbol;

		Value* args;
		ObjClass* classObj;
		Method* method;

	CASE_CODE(SUPER_0)
	CASE_CODE(SUPER_1)
	CASE_CODE(SUPER_2)
	CASE_CODE(SUPER_3)
	CASE_CODE(SUPER_4)
	CASE_CODE(SUPER_5)
	CASE_CODE(SUPER_6)
	CASE_CODE(SUPER_7)
	CASE_CODE(SUPER_8)
	CASE_CODE(SUPER_9)
	CASE_CODE(SUPER_10)
	CASE_CODE(SUPER_11)
	CASE_CODE(SUPER_12)
	CASE_CODE(SUPER_13)
	CASE_CODE(SUPER_14)
	CASE_CODE(SUPER_15)
	CASE_CODE(SUPER_16)
		numArgs = instruction - CODE_SUPER_0 + 1;
		symbol = READ_SHORT();

		args = fiber->stackTop - numArgs;

		classObj = AS_CLASS(fn->constants.data[READ_SHORT()]);
		goto completeCall;

	completeCall:
		if (symbol >= classObj->methods.count || METHOD_NONE == (method = &classObj->methods.data[symbol])->type) {
			methodNotFound(vm, classObj, symbol);
			RUNTIME_ERROR();
		}

		switch (method->type) {
		case METHOD_PRIMITIVE:
			if (method->as.primitive(vm, args)) {
				fiber->stackTop -= numArgs - 1;
			}
			else {
				STORE_FRAME();

				fiber = vm->fiber;
				if (NULL == fiber)
					return WREN_RESULT_SUCCESS;
				if (wrenHasError(fiber))
					RUNTIME_ERROR();

				LOAD_FRAME();
			}
			break;
		case METHOD_FOREIGN:
			callForeign(vm, fiber, method->as.foreign, numArgs);
			if (wrenHasError(fiber))
				RUNTIME_ERROR();
			break;
		case METHOD_BLOCK:
			STORE_FRAME();
			wrenCallFunction(vm, fiber, (ObjClosure*)method->as.closure, numArgs);
			LOAD_FRAME();
			break;
		case METHOD_NONE: UNREACHABLE(); break;
		}
		DISPATCH();
	}

	CASE_CODE(LOAD_UPVALUE) {
		ObjUpvalue** upvalues = frame->closure->upvalues;
		PUSH(*upvalues[READ_BYTE()]->value);
		DISPATCH();
	}

	CASE_CODE(STORE_UPVALUE) {
		ObjUpvalue** upvalues = frame->closure->upvalues;
		*upvalues[READ_BYTE()]->value = PEEK();
		DISPATCH();
	}

	CASE_CODE(LOAD_MODULE_VAR) {
		PUSH(fn->module->variables.data[READ_SHORT()]);
		DISPATCH();
	}

	CASE_CODE(STORE_MODULE_VAR) {
		fn->module->variables.data[READ_SHORT()] = PEEK();
		DISPATCH();
	}

	CASE_CODE(STORE_FIELD_THIS) {
		u8_t field = READ_BYTE();
		Value receiver = stackStart[0];
		ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
		ObjInstance* instance = AS_INSTANCE(receiver);
		ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
		instance->fields[field] = PEEK();
		DISPATCH();
	}

	CASE_CODE(LOAD_FIELD) {
		u8_t field = READ_BYTE();
		Value receiver = POP();
		ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
		ObjInstance* instance = AS_INSTANCE(receiver);
		ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
		PUSH(instance->fields[field]);
		DISPATCH();
	}

	CASE_CODE(STORE_FIELD) {
		u8_t field = READ_BYTE();
		Value receiver = POP();
		ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
		ObjInstance* instance = AS_INSTANCE(receiver);
		ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
		instance->fields[field] = PEEK();
		DISPATCH();
	}

	CASE_CODE(JUMP) {
		u16_t offset = READ_SHORT();
		ip += offset;
		DISPATCH();
	}

	CASE_CODE(LOOP) {
		u16_t offset = READ_SHORT();
		ip -= offset;
		DISPATCH();
	}

	CASE_CODE(JUMP_IF) {
		u16_t offset = READ_SHORT();
		Value condition = POP();

		if (IS_FALSE(condition) || IS_NULL(condition))
			ip += offset;
		DISPATCH();
	}

	CASE_CODE(AND) {
		u16_t offset = READ_SHORT();
		Value condition = PEEK();

		if (IS_FALSE(condition) || IS_NULL(condition))
			ip += offset;
		else
			DROP();
		DISPATCH();
	}

	CASE_CODE(OR) {
		u16_t offset = READ_SHORT();
		Value condition = PEEK();

		if (IS_FALSE(condition) || IS_NULL(condition))
			DROP();
		else
			ip += offset;
		DISPATCH();
	}

	CASE_CODE(CLOSE_UPVALUE) {
		closeUpvalues(fiber, fiber->stackTop - 1);
		DROP();
		DISPATCH();
	}

	CASE_CODE(RETURN) {
		Value result = POP();
		--fiber->numFrames;

		closeUpvalues(fiber, stackStart);

		if (0 == fiber->numFrames) {
			if (NULL == fiber->caller) {
				fiber->stack[0] = result;
				fiber->stackTop = fiber->stack + 1;
				return WREN_RESULT_SUCCESS;
			}

			ObjFiber* resumingFiber = fiber->caller;
			fiber->caller = NULL;
			fiber = resumingFiber;

			fiber->stackTop[-1] = result;
		}
		else {
			stackStart[0] = result;
			fiber->stackTop = frame->stackStart + 1;
		}

		LOAD_FRAME();
		DISPATCH();
	}

	CASE_CODE(CONSTRUCT) {
		ASSERT(IS_CLASS(stackStart[0]), "`this` should be a class.");
		stackStart[0] = wrenNewInstance(vm, AS_CLASS(stackStart[0]));
		DISPATCH();
	}

	CASE_CODE(FOREIGN_CONSTRUCT) {
		ASSERT(IS_CLASS(stackStart[0]), "`this` should be a class.");
		createForeign(vm, fiber, stackStart);
		DISPATCH();
	}

	CASE_CODE(CLOSURE) {
		ObjFn* function = AS_FN(fn->constants.data[READ_SHORT()]);
		ObjClosure* closure = wrenNewClosure(vm, function);
		PUSH(OBJ_VAL(closure));

		for (int i = 0; i < function->numUpvalues; ++i) {
			u8_t isLocal = READ_BYTE();
			u8_t index = READ_BYTE();
			if (isLocal)
				closure->upvalues[i] = captureUpvalue(vm, fiber, frame->stackStart + index);
			else
				closure->upvalues[i] = frame->closure->upvalues[index];
		}
		DISPATCH();
	}

	CASE_CODE(CLASS) {
		createClass(vm, READ_BYTE(), NULL);
		if (wrenHasError(fiber))
			RUNTIME_ERROR();
		DISPATCH();
	}

	CASE_CODE(FOREIGN_CLASS) {
		createClass(vm, -1, fn->module);
		if (wrenHasError(fiber))
			RUNTIME_ERROR();
		DISPATCH();
	}

	CASE_CODE(METHOD_INSTANCE)
	CASE_CODE(METHOD_STATIC) {
		u16_t symbol = READ_SHORT();
		ObjClass* classObj = AS_CLASS(PEEK());
		Value method = PEEK2();
		bindMethod(vm, instruction, symbol, fn->module, classObj, method);
		if (wrenHasError(fiber))
			RUNTIME_ERROR();
		DROP(); DROP();
		DISPATCH();
	}

	CASE_CODE(END_MODULE) {
		vm->lastModule = fn->module;
		PUSH(NULL_VAL);
		DISPATCH();
	}

	CASE_CODE(IMPORT_MODULE) {
		PUSH(importModule(vm, fn->constants.data[READ_SHORT()]));
		if (wrenHasError(fiber))
			RUNTIME_ERROR();

		if (IS_CLOSURE(PEEK())) {
			STORE_FRAME();
			ObjClosure* closure = AS_CLOSURE(PEEK());
			wrenCallFunction(vm, fiber, closure, 1);
			LOAD_FRAME();
		}
		else {
			vm->lastModule = AS_MODULE(PEEK());
		}
		DISPATCH();
	}

	CASE_CODE(IMPORT_VARIABLE) {
		Value variable = fn->constants.data[READ_SHORT()];
		ASSERT(NULL != vm->lastModule, "Should have already imported module.");
		Value result = getModuleVariable(vm, vm->lastModule, variable);
		if (wrenHasError(fiber))
			RUNTIME_ERROR();

		PUSH(result);
		DISPATCH();
	}

	CASE_CODE(END)      UNREACHABLE();
	default:            break;
	}

#undef DISPATCH
#undef CASE_CODE
#undef INTERPRET_LOOP
#undef DEBUG_TRACE_INSTRUCTIONS
#undef RUNTIME_ERROR
#undef LOAD_FRAME
#undef STORE_FRAME
#undef READ_SHORT
#undef READ_BYTE
#undef PEEK2
#undef PEEK
#undef DROP
#undef POP
#undef PUSH

	UNREACHABLE();
	return WREN_RESULT_RUNTIME_ERROR;
}

static inline void validateApiSlot(WrenVM* vm, int slot) {
	ASSERT(slot >= 0, "Slot cannot be negative.");
	ASSERT(slot < wrenGetSlotCount(vm), "Not that many slots.");
}

static inline void setSlot(WrenVM* vm, int slot, Value value) {
	validateApiSlot(vm, slot);
	vm->apiStack[slot] = value;
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

void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign) {
	int symbol = wrenSymbolTableFind(&vm->methodNames, "<finalize>", 10);
	ASSERT(-1 != symbol, "Should have defined <finalize> symbol.");

	if (-1 == symbol)
		return;

	ObjClass* classObj = foreign->obj.classObj;
	if (symbol >= classObj->methods.count)
		return;

	Method* method = &classObj->methods.data[symbol];
	if (METHOD_NONE == method->type)
		return;

	ASSERT(METHOD_FOREIGN == method->type, "Finalizer should be foreign.");

	WrenFinalizerFn finalizer = (WrenFinalizerFn)method->as.foreign;
	finalizer(foreign->data);
}

WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature) {
	ASSERT(NULL != signature, "Signature cannot be NULL.");

	int signatureLength = (int)strlen(signature);
	ASSERT(signatureLength > 0, "Signature cannot be empty.");

	int numParams = 0;
	if (')' == signature[signatureLength - 1]) {
		for (int i = signatureLength - 1; i > 0 && '(' != signature[i]; --i) {
			if ('_' == signature[i])
				++numParams;
		}
	}

	if ('[' == signature[0]) {
		for (int i = 0; i < signatureLength && ']' != signature[i]; ++i)
			if ('_' == signature[i])
				++numParams;
	}

	int method = wrenSymbolTableEnsure(vm, &vm->methodNames, signature, signatureLength);
	ObjFn* fn = wrenNewFunction(vm, NULL, numParams + 1);

	WrenHandle* value = wrenMakeHandle(vm, OBJ_VAL(fn));
	value->value = OBJ_VAL(wrenNewClosure(vm, fn));

	wrenByteBufferWrite(vm, &fn->code, (u8_t)(CODE_CALL_0 + numParams));
	wrenByteBufferWrite(vm, &fn->code, (method >> 8) & 0xff);
	wrenByteBufferWrite(vm, &fn->code, method & 0xff);
	wrenByteBufferWrite(vm, &fn->code, CODE_RETURN);
	wrenByteBufferWrite(vm, &fn->code, CODE_END);
	wrenIntBufferFill(vm, &fn->debug->sourceLines, 0, 5);
	wrenFunctionBindName(vm, fn, signature, signatureLength);

	return value;
}

WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method) {
	ASSERT(NULL != method, "Method cannot be NULL.");
	ASSERT(IS_CLOSURE(method->value), "Method must be a method handle.");
	ASSERT(NULL != vm->fiber, "Must set up arguments for call first.");
	ASSERT(NULL != vm->apiStack, "Must set up arguments for call first.");
	ASSERT(0 == vm->fiber->numFrames, "Can not call from a foreign method.");

	ObjClosure* closure = AS_CLOSURE(method->value);
	ASSERT(vm->fiber->stackTop - vm->fiber->stack >= closure->fn->arity,
			"Stack must have enough arguments for method.");

	vm->apiStack = NULL;
	vm->fiber->stackTop = &vm->fiber->stack[closure->fn->maxSlots];

	wrenCallFunction(vm, vm->fiber, closure, 0);
	WrenInterpretResult result = runInterpreter(vm, vm->fiber);

	if (NULL != vm->fiber)
		vm->apiStack = vm->fiber->stack;

	return result;
}

WrenHandle* wrenMakeHandle(WrenVM* vm, Value value) {
	if (IS_OBJ(value))
		wrenPushRoot(vm, AS_OBJ(value));

	WrenHandle* handle = ALLOCATE(vm, WrenHandle);
	handle->value = value;

	if (IS_OBJ(value))
		wrenPopRoot(vm);

	if (NULL != vm->handles)
		vm->handles->prev = handle;
	handle->prev = NULL;
	handle->next = vm->handles;
	vm->handles = handle;

	return handle;
}

void wrenReleaseHandle(WrenVM* vm, WrenHandle* handle) {
	ASSERT(NULL != handle, "Handle cannot be NULL.");

	if (vm->handles == handle)
		vm->handles = handle->next;

	if (NULL != handle->prev)
		handle->prev->next = handle->next;
	if (NULL != handle->next)
		handle->next->prev = handle->prev;

	handle->prev = NULL;
	handle->next = NULL;
	handle->value = NULL_VAL;
	DEALLOCATE(vm, handle);
}

WrenInterpretResult wrenInterpret(WrenVM* vm, const char* module, const char* sourceCode) {
	ObjClosure* closure = wrenCompileSource(vm, module, sourceCode, false, true);
	if (NULL == closure)
		return WREN_RESULT_COMPILE_ERROR;

	WREN_PUSH_ROOT(vm, closure);
	ObjFiber* fiber = wrenNewFiber(vm, closure);
	WREN_POP_ROOT(vm);

	return runInterpreter(vm, fiber);
}

ObjClosure* wrenCompileSource(WrenVM* vm,
		const char* module, const char* sourceCode, bool isExpression, bool printErrors) {
	Value nameValue = NULL_VAL;
	if (NULL != module) {
		nameValue = wrenNewString(vm, module);
		WREN_PUSH_ROOT(vm, AS_OBJ(nameValue));
	}

	ObjClosure* closure = compileInModule(vm, nameValue, sourceCode, isExpression, printErrors);

	if (NULL != module)
		WREN_POP_ROOT(vm);

	return closure;
}

Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName) {
	ObjModule* module = getModule(vm, moduleName);
	if (NULL == module) {
		vm->fiber->error = wrenStringFormat(vm, "Module `@` is not loaded.", moduleName);
		return NULL_VAL;
	}

	return getModuleVariable(vm, module, variableName);
}

Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name) {
	int symbol = wrenSymbolTableFind(&module->variableNames, name, strlen(name));
	return module->variables.data[symbol];
}

int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name, sz_t length, int lineno) {
	if (module->variables.count >= WREN_MAX_MODULE_VARS)
		return -2;

	wrenValueBufferWrite(vm, &module->variables, NUM_VAL(lineno));
	return wrenSymbolTableAdd(vm, &module->variableNames, name, length);
}

int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name, sz_t length, Value value) {
	if (module->variables.count >= WREN_MAX_MODULE_VARS)
		return -2;

	if (IS_OBJ(value))
		WREN_PUSH_ROOT(vm, AS_OBJ(value));

	int symbol = wrenSymbolTableFind(&module->variableNames, name, length);
	if (-1 == symbol) {
		symbol = wrenSymbolTableAdd(vm, &module->variableNames, name, length);
		wrenValueBufferWrite(vm, &module->variables, value);
	}
	else if (IS_NUM(module->variables.data[symbol])) {
		module->variables.data[symbol] = value;
	}
	else {
		symbol = -1;
	}

	if (IS_OBJ(value))
		WREN_POP_ROOT(vm);

	return symbol;
}

void wrenPushRoot(WrenVM* vm, Obj* obj) {
	ASSERT(NULL != obj, "Cannot root NULL.");
	ASSERT(vm->numTempRoots < WREN_MAX_TEMP_ROOTS, "Too many temporary roots.");

	vm->tempRoots[vm->numTempRoots++] = obj;
}

void wrenPopRoot(WrenVM* vm) {
	ASSERT(vm->numTempRoots > 0, "No temporary roots to release.");
	--vm->numTempRoots;
}

int wrenGetSlotCount(WrenVM* vm) {
	if (NULL == vm->apiStack)
		return 0;

	return (int)(vm->fiber->stackTop - vm->apiStack);
}

void wrenEnsureSlots(WrenVM* vm, int numSlots) {
	if (NULL == vm->apiStack) {
		vm->fiber = wrenNewFiber(vm, NULL);
		vm->apiStack = vm->fiber->stack;
	}

	int currentSize = (int)(vm->fiber->stackTop - vm->apiStack);
	if (currentSize >= numSlots)
		return;

	int needed = (int)(vm->apiStack - vm->fiber->stack) + numSlots;
	wrenEnsureStack(vm, vm->fiber, needed);

	vm->fiber->stackTop = vm->apiStack + numSlots;
}

WrenType wrenGetSlotType(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);

	Value api_value = vm->apiStack[slot];
	if (IS_BOOL(api_value))
		return WREN_TYPE_BOOL;
	if (IS_NUM(api_value))
		return WREN_TYPE_NUM;
	if (IS_FOREIGN(api_value))
		return WREN_TYPE_FOREIGN;
	if (IS_LIST(api_value))
		return WREN_TYPE_LIST;
	if (IS_NULL(api_value))
		return WREN_TYPE_NULL;
	if (IS_STRING(api_value))
		return WREN_TYPE_STRING;

	return WREN_TYPE_UNKNOWN;
}

bool wrenGetSlotBool(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	ASSERT(IS_BOOL(vm->apiStack[slot]), "Slot must hold a bool.");

	return AS_BOOL(vm->apiStack[slot]);
}

const char* wrenGetSlotBytes(WrenVM* vm, int slot, int* length) {
	validateApiSlot(vm, slot);
	ASSERT(IS_STRING(vm->apiStack[slot]), "Slot must hold a string.");

	ObjString* string = AS_STRING(vm->apiStack[slot]);
	*length = string->length;
	return string->value;
}

double wrenGetSlotDouble(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	ASSERT(IS_NUM(vm->apiStack[slot]), "Slot must hold a number.");

	return AS_NUM(vm->apiStack[slot]);
}

void* wrenGetSlotForeign(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	ASSERT(IS_FOREIGN(vm->apiStack[slot]), "Slot must hold a foreign instance.");

	return AS_FOREIGN(vm->apiStack[slot])->data;
}

const char* wrenGetSlotString(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	ASSERT(IS_STRING(vm->apiStack[slot]), "Slot must hold a string.");

	return AS_CSTRING(vm->apiStack[slot]);
}

WrenHandle* wrenGetSlotHandle(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	return wrenMakeHandle(vm, vm->apiStack[slot]);
}

void wrenSetSlotBool(WrenVM* vm, int slot, bool value) {
	setSlot(vm, slot, BOOL_VAL(value));
}

void wrenSetSlotBytes(WrenVM* vm, int slot, const char* bytes, sz_t length) {
	ASSERT(NULL != bytes, "Bytes array cannot be NULL.");
	setSlot(vm, slot, wrenNewStringLength(vm, bytes, length));
}

void wrenSetSlotDouble(WrenVM* vm, int slot, double value) {
	setSlot(vm, slot, NUM_VAL(value));
}

void* wrenSetSlotNewForeign(WrenVM* vm, int slot, int classSlot, sz_t size) {
	validateApiSlot(vm, slot);
	validateApiSlot(vm, classSlot);
	ASSERT(IS_CLASS(vm->apiStack[classSlot]), "Slot must hold a class.");

	ObjClass* classObj = AS_CLASS(vm->apiStack[classSlot]);
	ASSERT(-1 == classObj->numFields, "Class must be a foreign class.");

	ObjForeign* foreign = wrenNewForeign(vm, classObj, size);
	vm->apiStack[slot] = OBJ_VAL(foreign);

	return (void*)foreign->data;
}

void wrenSetSlotNewList(WrenVM* vm, int slot) {
	setSlot(vm, slot, OBJ_VAL(wrenNewList(vm, 0)));
}

void wrenSetSlotNull(WrenVM* vm, int slot) {
	setSlot(vm, slot, NULL_VAL);
}

void wrenSetSlotString(WrenVM* vm, int slot, const char* text) {
	ASSERT(NULL != text, "String cannot be NULL.");
	setSlot(vm, slot, wrenNewString(vm, text));
}

void wrenSetSlotHandle(WrenVM* vm, int slot, WrenHandle* handle) {
	ASSERT(NULL != handle, "Handle canno be NULL.");
	setSlot(vm, slot, handle->value);
}

int wrenGetListCount(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	ASSERT(IS_LIST(vm->apiStack[slot]), "Slot must hold a list.");

	ValueBuffer elements = AS_LIST(vm->apiStack[slot])->elements;
	return elements.count;
}

void wrenGetListElement(WrenVM* vm, int listSlot, int index, int elementSlot) {
	validateApiSlot(vm, listSlot);
	validateApiSlot(vm, elementSlot);
	ASSERT(IS_LIST(vm->apiStack[listSlot]), "Slot must hold a list.");

	ValueBuffer elements = AS_LIST(vm->apiStack[listSlot])->elements;
	vm->apiStack[elementSlot] = elements.data[index];
}

void wrenInsertInList(WrenVM* vm, int listSlot, int index, int elementSlot) {
	validateApiSlot(vm, listSlot);
	validateApiSlot(vm, elementSlot);
	ASSERT(IS_LIST(vm->apiStack[listSlot]), "Must insert into a list.");

	ObjList* list = AS_LIST(vm->apiStack[listSlot]);

	if (index < 0)
		index = list->elements.count + index + 1;
	ASSERT(index <= list->elements.count, "Index out of bounds.");

	wrenListInsert(vm, list, vm->apiStack[elementSlot], index);
}

void wrenGetVariable(WrenVM* vm, const char* module, const char* name, int slot) {
	ASSERT(NULL != module, "Module cannot be NULL.");
	ASSERT(NULL != name, "Variable name cannot be NULL.");
	validateApiSlot(vm, slot);

	Value moduleName = wrenStringFormat(vm, "$", module);
	WREN_PUSH_ROOT(vm, AS_OBJ(moduleName));

	ObjModule* moduleObj = getModule(vm, moduleName);
	ASSERT(NULL != moduleObj, "Could not find module.");

	WREN_POP_ROOT(vm);

	int variableSlot = wrenSymbolTableFind(&moduleObj->variableNames, name, strlen(name));
	ASSERT(-1 != variableSlot, "Could not find variable.");

	setSlot(vm, slot, moduleObj->variables.data[variableSlot]);
}

void wrenAbortFiber(WrenVM* vm, int slot) {
	validateApiSlot(vm, slot);
	vm->fiber->error = vm->apiStack[slot];
}

void* wrenGetUserData(WrenVM* vm) {
	return vm->config.userData;
}

void wrenSetUserData(WrenVM* vm, void* userData) {
	vm->config.userData = userData;
}
