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
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "core.h"
#include "primitive.h"
#include "value.h"

#include "core.wren.inc"

#define DEF_FN_CALL(numArgs)\
	DEF_PRIMITIVE(fn_call##numArgs) {\
		call(vm, args, numArgs);\
		return false;\
	}

#define DEF_NUM_INFIX(name, op, type)\
	DEF_PRIMITIVE(num_##name) {\
		if (!validateNum(vm, args[1], "Right operand"))\
			return false;\
		RETURN_##type(AS_NUM(args[0]) op AS_NUM(args[1]));\
	}

#define DEF_NUM_BITWISE(name, op)\
	DEF_PRIMITIVE(num_bitwise##name) {\
		if (!validateNum(vm, args[1], "Right operand"))\
			return false;\
		u32_t left = (u32_t)AS_NUM(args[0]);\
		u32_t right = (u32_t)AS_NUM(args[1]);\
		RETURN_NUM(left op right);\
	}

#define DEF_NUM_FN(name, fn)\
	DEF_PRIMITIVE(num_##name) {\
		RETURN_NUM(fn(AS_NUM(args[0])));\
	}

static bool runFiber(WrenVM* vm, ObjFiber* fiber,
		Value* args, bool isCall, bool hasValue, const char* verb) {
	if (wrenHasError(fiber))
		RETURN_ERROR_FMT("Cannot $ an aborted fiber.", verb);

	if (isCall) {
		if (NULL != fiber->caller)
			RETURN_ERROR("Fiber has already been called.");
		if (FIBER_ROOT == fiber->state)
			RETURN_ERROR("Cannot call root fiber.");
		fiber->caller = vm->fiber;
	}

	if (0 == fiber->numFrames)
		RETURN_ERROR_FMT("Cannot $ a finished fiber.", verb);

	if (hasValue)
		--vm->fiber->stackTop;
	if (1 == fiber->numFrames && fiber->frames[0].ip == fiber->frames[0].closure->fn->code.data) {
		if (1 == fiber->frames[0].closure->fn->arity) {
			fiber->stackTop[0] = hasValue ? args[0] : NULL_VAL;
			++fiber->stackTop;
		}
	}
	else {
		fiber->stackTop[-1] = hasValue ? args[1] : NULL_VAL;
	}

	vm->fiber = fiber;
	return false;
}

static void call(WrenVM* vm, Value* args, int numArgs) {
	if (AS_CLOSURE(args[0])->fn->arity > numArgs) {
		vm->fiber->error = CONST_STRING(vm, "Function expects more arguments.");
		return;
	}

	wrenCallFunction(vm, vm->fiber, AS_CLOSURE(args[0]), numArgs + 1);
}

static ObjClass* defineClass(WrenVM* vm, ObjModule* module, const char* name) {
	ObjString* nameString = AS_STRING(wrenNewString(vm, name));
	WREN_PUSH_ROOT(vm, nameString);

	ObjClass* classObj = wrenNewSingleClass(vm, 0, nameString);
	wrenDefineVariable(vm, module, name, nameString->length, OBJ_VAL(classObj));

	WREN_POP_ROOT(vm);
	return classObj;
}

DEF_PRIMITIVE(bool_not) {
	RETURN_BOOL(!AS_BOOL(args[0]));
}

DEF_PRIMITIVE(bool_toString) {
	if (AS_BOOL(args[0]))
		RETURN_VAL(CONST_STRING(vm, "true"));
	else
		RETURN_VAL(CONST_STRING(vm, "false"));
}

DEF_PRIMITIVE(class_name) {
	RETURN_OBJ(AS_CLASS(args[0])->name);
}

DEF_PRIMITIVE(class_supertype) {
	ObjClass* classObj = AS_CLASS(args[0]);

	if (NULL == classObj->superclass)
		RETURN_NULL;
	RETURN_OBJ(classObj->superclass);
}

DEF_PRIMITIVE(class_toString) {
	RETURN_OBJ(AS_CLASS(args[0])->name);
}

DEF_PRIMITIVE(fiber_new) {
	if (!validateFn(vm, args[1], "Argument"))
		return false;

	ObjClosure* closure = AS_CLOSURE(args[1]);
	if (closure->fn->arity > 1)
		RETURN_ERROR("Function cannot take more than one parameter.");
	RETURN_OBJ(wrenNewFiber(vm, closure));
}

DEF_PRIMITIVE(fiber_abort) {
	vm->fiber->error = args[1];
	return IS_NULL(args[1]);
}

DEF_PRIMITIVE(fiber_call) {
	return runFiber(vm, AS_FIBER(args[0]), args, true, false, "call");
}

DEF_PRIMITIVE(fiber_call1) {
	return runFiber(vm, AS_FIBER(args[0]), args, true, true, "call");
}

DEF_PRIMITIVE(fiber_current) {
	RETURN_OBJ(vm->fiber);
}

DEF_PRIMITIVE(fiber_error) {
	RETURN_VAL(AS_FIBER(args[0])->error);
}

DEF_PRIMITIVE(fiber_isDone) {
	ObjFiber* runFiber = AS_FIBER(args[0]);
	RETURN_BOOL(0 == runFiber->numFrames || wrenHasError(runFiber));
}

DEF_PRIMITIVE(fiber_suspend) {
	vm->fiber = NULL;
	vm->apiStack = NULL;
	return false;
}

DEF_PRIMITIVE(fiber_transfer) {
	return runFiber(vm, AS_FIBER(args[0]), args, false, false, "transfer to");
}

DEF_PRIMITIVE(fiber_transfer1) {
	return runFiber(vm, AS_FIBER(args[0]), args, false, true, "transfer to");
}

DEF_PRIMITIVE(fiber_transferError) {
	runFiber(vm, AS_FIBER(args[0]), args, false, true, "transfer to");
	vm->fiber->error = args[1];
	return false;
}

DEF_PRIMITIVE(fiber_try) {
	runFiber(vm, AS_FIBER(args[0]), args, true, false, "try");
	if (!wrenHasError(vm->fiber))
		vm->fiber->state = FIBER_TRY;
	return false;
}

DEF_PRIMITIVE(fiber_yield) {
	ObjFiber* currentFiber = vm->fiber;
	vm->fiber = currentFiber->caller;

	currentFiber->caller = NULL;
	currentFiber->state = FIBER_OTHER;

	if (NULL != vm->fiber)
		vm->fiber->stackTop[-1] = NULL_VAL;
	return false;
}

DEF_PRIMITIVE(fiber_yield1) {
	ObjFiber* currentFiber = vm->fiber;
	vm->fiber = currentFiber->caller;

	currentFiber->caller = NULL;
	currentFiber->state = FIBER_OTHER;

	if (NULL != vm->fiber) {
		vm->fiber->stackTop[-1] = args[1];
		--currentFiber->stackTop;
	}
	return false;
}

DEF_PRIMITIVE(fn_new) {
	if (!validateFn(vm, args[1], "Argument"))
		return false;
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(fn_arity) {
	RETURN_NUM(AS_CLOSURE(args[0])->fn->arity);
}

DEF_FN_CALL(0)
DEF_FN_CALL(1)
DEF_FN_CALL(2)
DEF_FN_CALL(3)
DEF_FN_CALL(4)
DEF_FN_CALL(5)
DEF_FN_CALL(6)
DEF_FN_CALL(7)
DEF_FN_CALL(8)
DEF_FN_CALL(9)
DEF_FN_CALL(10)
DEF_FN_CALL(11)
DEF_FN_CALL(12)
DEF_FN_CALL(13)
DEF_FN_CALL(14)
DEF_FN_CALL(15)
DEF_FN_CALL(16)

DEF_PRIMITIVE(fn_toString) {
	RETURN_VAL(CONST_STRING(vm, "<fn>"));
}

DEF_PRIMITIVE(list_filled) {
	if (!validateInt(vm, args[1], "Size"))
		return false;
	if (AS_NUM(args[1]) < 0)
		RETURN_ERROR("Size cannot be negative.");

	u32_t size = (u32_t)AS_NUM(args[1]);
	ObjList* list = wrenNewList(vm, size);

	Value filled_value = args[2];
	for (u32_t i = 0; i < size; ++i)
		list->elements.data[i] = filled_value;

	RETURN_OBJ(list);
}

DEF_PRIMITIVE(list_new) {
	RETURN_OBJ(wrenNewList(vm, 0));
}

DEF_PRIMITIVE(list_add) {
	wrenValueBufferWrite(vm, &AS_LIST(args[0])->elements, args[1]);
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(list_addCore) {
	wrenValueBufferWrite(vm, &AS_LIST(args[0])->elements, args[1]);
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(list_clear) {
	wrenValueBufferClear(vm, &AS_LIST(args[0])->elements);
	RETURN_NULL;
}

DEF_PRIMITIVE(list_count) {
	RETURN_NUM(AS_LIST(args[0])->elements.count);
}

DEF_PRIMITIVE(list_insert) {
	ObjList* list = AS_LIST(args[0]);
	u32_t index = validateIndex(vm, args[1], list->elements.count + 1, "Index");
	if (UINT32_MAX == index)
		return false;

	wrenListInsert(vm, list, args[2], index);
	RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(list_iterate) {
	ObjList* list = AS_LIST(args[0]);
	if (IS_NULL(args[1])) {
		if (0 == list->elements.count)
			RETURN_FALSE;
		RETURN_NUM(0);
	}

	if (!validateFn(vm, args[1], "Iterator"))
		return false;

	double index = AS_NUM(args[1]);
	if (index < 0 || index >= list->elements.count - 1)
		RETURN_FALSE;
	RETURN_NUM(index + 1);
}

DEF_PRIMITIVE(list_iteratorValue) {
	ObjList* list = AS_LIST(args[0]);
	u32_t index = validateIndex(vm, args[1], list->elements.count, "Iterator");
	if (UINT32_MAX == index)
		return false;

	RETURN_VAL(list->elements.data[index]);
}

DEF_PRIMITIVE(list_removeAt) {
	ObjList* list = AS_LIST(args[0]);
	u32_t index = validateIndex(vm, args[1], list->elements.count, "Index");
	if (UINT32_MAX == index)
		return false;

	RETURN_VAL(wrenListRemoveAt(vm, list, index));
}

DEF_PRIMITIVE(list_subscript) {
	ObjList* list = AS_LIST(args[0]);
	if (IS_NUM(args[1])) {
		u32_t index = validateIndex(vm, args[1], list->elements.count, "Subscript");
		if (UINT32_MAX == index)
			return false;

		RETURN_VAL(list->elements.data[index]);
	}

	if (!IS_RANGE(args[1]))
		RETURN_ERROR("Subscript must be a number or a range.");

	int step;
	u32_t count = list->elements.count;
	u32_t start = calculateRange(vm, AS_RANGE(args[1]), &count, &step);
	if (UINT32_MAX == start)
		return false;

	ObjList* result = wrenNewList(vm, count);
	for (u32_t i = 0; i < count; ++i)
		result->elements.data[i] = list->elements.data[start + i * step];

	RETURN_OBJ(result);
}

DEF_PRIMITIVE(list_subscriptSetter) {
	ObjList* list = AS_LIST(args[0]);
	u32_t index = validateIndex(vm, args[1], list->elements.count, "Subscript");
	if (UINT32_MAX == index)
		return false;

	list->elements.data[index] = args[2];
	RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(map_new) {
	RETURN_OBJ(wrenNewMap(vm));
}

DEF_PRIMITIVE(map_subscript) {
	if (!validateKey(vm, args[1]))
		return false;

	ObjMap* map = AS_MAP(args[0]);
	Value value = wrenMapGet(map, args[1]);
	if (IS_UNDEFINED(value))
		RETURN_NULL;

	RETURN_VAL(value);
}

DEF_PRIMITIVE(map_subscriptSetter) {
	if (!validateKey(vm, args[1]))
		return false;

	wrenMapSet(vm, AS_MAP(args[0]), args[1], args[2]);
	RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(map_addCore) {
	if (!validateKey(vm, args[1]))
		return false;

	wrenMapSet(vm, AS_MAP(args[0]), args[1], args[2]);
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(map_clear) {
	wrenMapClear(vm, AS_MAP(args[0]));
	RETURN_NULL;
}

DEF_PRIMITIVE(map_containsKey) {
	if (!validateKey(vm, args[1]))
		return false;

	RETURN_BOOL(!IS_UNDEFINED(wrenMapGet(AS_MAP(args[0]), args[1])));
}

DEF_PRIMITIVE(map_count) {
	RETURN_NUM(AS_MAP(args[0])->count);
}

DEF_PRIMITIVE(map_iterate) {
	ObjMap* map = AS_MAP(args[0]);
	if (0 == map->count)
		RETURN_FALSE;

	u32_t index = 0;
	if (!IS_NULL(args[1])) {
		if (!validateInt(vm, args[1], "Iterator"))
			return false;

		if (AS_NUM(args[1]) < 0)
			RETURN_FALSE;
		index = (u32_t)AS_NUM(args[1]);

		if (index >= map->capacity)
			RETURN_FALSE;

		++index;
	}

	for (; index < map->capacity; ++index) {
		if (!IS_UNDEFINED(map->entries[index].key))
			RETURN_NUM(index);
	}

	RETURN_FALSE;
}

DEF_PRIMITIVE(map_remove) {
	if (!validateKey(vm, args[1]))
		return false;

	RETURN_VAL(wrenMapRemoveKey(vm, AS_MAP(args[0]), args[1]));
}

DEF_PRIMITIVE(map_keyIteratorValue) {
	ObjMap* map = AS_MAP(args[0]);
	u32_t index = validateIndex(vm, args[1], map->capacity, "Iterator");
	if (UINT32_MAX == index)
		return false;

	MapEntry* entry = &map->entries[index];
	if (IS_UNDEFINED(entry->key))
		RETURN_ERROR("Invalid map iterator.");

	RETURN_VAL(entry->key);
}

DEF_PRIMITIVE(map_valueIteratorValue) {
	ObjMap* map = AS_MAP(args[0]);
	u32_t index = validateIndex(vm, args[1], map->capacity, "Iterator");
	if (UINT32_MAX == index)
		return false;

	MapEntry* entry = &map->entries[index];
	if (IS_UNDEFINED(entry->key))
		RETURN_ERROR("Invalid map iterator.");

	RETURN_VAL(entry->value);
}

DEF_PRIMITIVE(null_not) {
	RETURN_VAL(TRUE_VAL);
}

DEF_PRIMITIVE(null_toString) {
	RETURN_VAL(CONST_STRING(vm, "null"));
}

DEF_PRIMITIVE(num_fromString) {
	if (!validateString(vm, args[1], "Argument"))
		return false;

	ObjString* string = AS_STRING(args[1]);
	if (0 == string->length)
		RETURN_NULL;

	errno = 0;
	char* end;
	double number = strtod(string->value, &end);

	while ('\0' != *end && isspace((unsigned char)*end))
		++end;
	if (ERANGE == errno)
		RETURN_ERROR("Number literal is too large.");

	if (end < string->value + string->length)
		RETURN_NULL;

	RETURN_NUM(number);
}

DEF_PRIMITIVE(num_pi) {
	RETURN_NUM(3.14159265358979323846);
}

DEF_NUM_INFIX(minus,                        -,  NUM)
DEF_NUM_INFIX(plus,                         +,  NUM)
DEF_NUM_INFIX(multiply,                     *,  NUM)
DEF_NUM_INFIX(divide,                       /,  NUM)
DEF_NUM_INFIX(lt,                           <,  BOOL)
DEF_NUM_INFIX(gt,                           >,  BOOL)
DEF_NUM_INFIX(lte,                          <=, BOOL)
DEF_NUM_INFIX(gte,                          >=, BOOL)

DEF_NUM_BITWISE(And,                        &)
DEF_NUM_BITWISE(Or,                         |)
DEF_NUM_BITWISE(Xor,                        ^)
DEF_NUM_BITWISE(LeftShift,                  <<)
DEF_NUM_BITWISE(RightShift,                 >>)

DEF_NUM_FN(abs,                             fabs)
DEF_NUM_FN(acos,                            acos)
DEF_NUM_FN(asin,                            asin)
DEF_NUM_FN(atan,                            atan)
DEF_NUM_FN(ceil,                            ceil)
DEF_NUM_FN(cos,                             cos)
DEF_NUM_FN(floor,                           floor)
DEF_NUM_FN(negate,                          -)
DEF_NUM_FN(round,                           round)
DEF_NUM_FN(sin,                             sin)
DEF_NUM_FN(sqrt,                            sqrt)
DEF_NUM_FN(tan,                             tan)
DEF_NUM_FN(log,                             log)

DEF_PRIMITIVE(num_mod) {
	if (!validateNum(vm, args[1], "Right operand"))
		return false;
	RETURN_NUM(fmod(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_PRIMITIVE(num_eqeq) {
	if (!IS_NUM(args[1]))
		RETURN_FALSE;
	RETURN_BOOL(AS_NUM(args[0]) == AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_bangeq) {
	if (!IS_NUM(args[1]))
		RETURN_TRUE;
	RETURN_BOOL(AS_NUM(args[0]) != AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_bitwiseNot) {
	RETURN_NUM(~(u32_t)AS_NUM(args[0]));
}

DEF_PRIMITIVE(num_dotDot) {
	if (!validateNum(vm, args[1], "Right hand side of range"))
		return false;

	double from = AS_NUM(args[0]);
	double to = AS_NUM(args[1]);
	RETURN_VAL(wrenNewRange(vm, from, to, true));
}

DEF_PRIMITIVE(num_dotDotDot) {
	if (!validateNum(vm, args[1], "Right hand side of range"))
		return false;

	double from = AS_NUM(args[0]);
	double to = AS_NUM(args[1]);
	RETURN_VAL(wrenNewRange(vm, from, to, false));
}

DEF_PRIMITIVE(num_atan2) {
	RETURN_NUM(atan2(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_PRIMITIVE(num_pow) {
	RETURN_NUM(pow(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_PRIMITIVE(num_fraction) {
	double dummy;
	RETURN_NUM(modf(AS_NUM(args[0]), &dummy));
}

DEF_PRIMITIVE(num_isInfinity) {
	RETURN_BOOL(isinf(AS_NUM(args[0])));
}

DEF_PRIMITIVE(num_isInteger) {
	double value = AS_NUM(args[0]);
	if (isnan(value) || isinf(value))
		RETURN_FALSE;
	RETURN_BOOL(trunc(value) == value);
}

DEF_PRIMITIVE(num_isNan) {
	RETURN_BOOL(isnan(AS_NUM(args[0])));
}

DEF_PRIMITIVE(num_sign) {
	double value = AS_NUM(args[0]);
	if (value > 0)
		RETURN_NUM(1);
	else if (value < 0)
		RETURN_NUM(-1);
	else
		RETURN_NUM(0);
}

DEF_PRIMITIVE(num_largest) {
	RETURN_NUM(DBL_MAX);
}

DEF_PRIMITIVE(num_smallest) {
	RETURN_NUM(DBL_MIN);
}

DEF_PRIMITIVE(num_toString) {
	RETURN_VAL(wrenNumToString(vm, AS_NUM(args[0])));
}

DEF_PRIMITIVE(num_truncate) {
	double integer;
	modf(AS_NUM(args[0]), &integer);
	RETURN_NUM(integer);
}

DEF_PRIMITIVE(object_same) {
	RETURN_BOOL(wrenValuesEqual(args[1], args[2]));
}

DEF_PRIMITIVE(object_not) {
	RETURN_VAL(FALSE_VAL);
}

DEF_PRIMITIVE(object_eqeq) {
	RETURN_BOOL(wrenValuesEqual(args[0], args[1]));
}

DEF_PRIMITIVE(object_bangeq) {
	RETURN_BOOL(!wrenValuesEqual(args[0], args[1]));
}

DEF_PRIMITIVE(object_is) {
	if (!IS_CLASS(args[1]))
		RETURN_ERROR("Right operand must be a class.");

	ObjClass* classObj = wrenGetClass(vm, args[0]);
	ObjClass* baseClassObj = AS_CLASS(args[1]);

	do {
		if (baseClassObj == classObj)
			RETURN_BOOL(true);

		classObj = classObj->superclass;
	} while (NULL != classObj);

	RETURN_BOOL(false);
}

DEF_PRIMITIVE(object_toString) {
	Obj* obj = AS_OBJ(args[0]);
	Value className = OBJ_VAL(obj->classObj->name);
	RETURN_VAL(wrenStringFormat(vm, "instance of @", className));
}

DEF_PRIMITIVE(object_type) {
	RETURN_OBJ(wrenGetClass(vm, args[0]));
}

DEF_PRIMITIVE(range_from) {
	RETURN_NUM(AS_RANGE(args[0])->from);
}

DEF_PRIMITIVE(range_to) {
	RETURN_NUM(AS_RANGE(args[0])->to);
}

DEF_PRIMITIVE(range_min) {
	ObjRange* range = AS_RANGE(args[0]);
	RETURN_NUM(fmin(range->from, range->to));
}

DEF_PRIMITIVE(range_max) {
	ObjRange* range = AS_RANGE(args[0]);
	RETURN_NUM(fmax(range->from, range->to));
}

DEF_PRIMITIVE(range_isInclusive) {
	RETURN_BOOL(AS_RANGE(args[0])->isInclusive);
}

DEF_PRIMITIVE(range_iterate) {
	ObjRange* range = AS_RANGE(args[0]);

	if (range->from == range->to && !range->isInclusive)
		RETURN_FALSE;

	if (IS_NULL(args[1]))
		RETURN_NUM(range->from);

	if (!validateNum(vm, args[1], "Iterator"))
		return false;

	double iterator = AS_NUM(args[1]);
	if (range->from < range->to) {
		++iterator;
		if (iterator > range->to)
			RETURN_FALSE;
	}
	else {
		--iterator;
		if (iterator < range->to)
			RETURN_FALSE;
	}

	if (!range->isInclusive && iterator == range->to)
		RETURN_FALSE;
	RETURN_NUM(iterator);
}

DEF_PRIMITIVE(range_iteratorValue) {
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(range_toString) {
	ObjRange* range = AS_RANGE(args[0]);

	Value from = wrenNumToString(vm, range->from);
	WREN_PUSH_ROOT(vm, AS_OBJ(from));

	Value to = wrenNumToString(vm, range->to);
	WREN_PUSH_ROOT(vm, AS_OBJ(to));

	Value result = wrenStringFormat(vm, "@$@", from, range->isInclusive ? ".." : "...", to);

	WREN_POP_ROOT(vm);
	WREN_POP_ROOT(vm);

	RETURN_VAL(result);
}

DEF_PRIMITIVE(string_fromCodePoint) {
	if (!validateInt(vm, args[1], "Code point"))
		return false;

	int codePoint = (int)AS_NUM(args[1]);
	if (codePoint < 0)
		RETURN_ERROR("Code point cannot be negative.");
	else if (codePoint > 0x10ffff)
		RETURN_ERROR("Code point cannot be greater than 0x10ffff.");

	RETURN_VAL(wrenStringFromCodePoint(vm, codePoint));
}

DEF_PRIMITIVE(string_fromByte) {
	if (!validateInt(vm, args[1], "Byte"))
		return false;

	int byte = (int)AS_NUM(args[1]);
	if (byte < 0)
		RETURN_ERROR("Byte cannot be negative.");
	else if (byte > 0xff)
		RETURN_ERROR("Byte cannot be greater than 0xff.");

	RETURN_VAL(wrenStringFromByte(vm, (u8_t)byte));
}

DEF_PRIMITIVE(string_byteAt) {
	ObjString* string = AS_STRING(args[0]);
	u32_t index = validateIndex(vm, args[1], string->length, "Index");
	if (UINT32_MAX == index)
		return false;

	RETURN_NUM((u8_t)string->value[index]);
}

DEF_PRIMITIVE(string_byteCount) {
	RETURN_NUM(AS_STRING(args[0])->length);
}

DEF_PRIMITIVE(string_codePointAt) {
	ObjString* string = AS_STRING(args[0]);
	u32_t index = validateIndex(vm, args[1], string->length, "Index");
	if (UINT32_MAX == index)
		return false;

	const u8_t* bytes = (u8_t*)string->value;
	if (0x80 == (bytes[index] & 0xc0))
		RETURN_NUM(-1);

	RETURN_NUM(wrenUtf8Decode((u8_t*)string->value + index, string->length - index));
}

DEF_PRIMITIVE(string_contains) {
	if (!validateString(vm, args[1], "Argument"))
		return false;

	ObjString* string = AS_STRING(args[0]);
	ObjString* search = AS_STRING(args[1]);
	RETURN_BOOL(UINT32_MAX != wrenStringFind(string, search, 0));
}

DEF_PRIMITIVE(string_endsWith) {
	if (!validateString(vm, args[1], "Argument"))
		return false;

	ObjString* string = AS_STRING(args[0]);
	ObjString* search = AS_STRING(args[1]);

	if (search->length > string->length)
		RETURN_FALSE;
	RETURN_BOOL(0 == memcmp(string->value + string->length - search->length, search->value, search->length));
}

DEF_PRIMITIVE(string_indexOf1) {
	if (!validateString(vm, args[1], "Argument"))
		return false;

	ObjString* string = AS_STRING(args[0]);
	ObjString* search = AS_STRING(args[1]);

	u32_t index = wrenStringFind(string, search, 0);
	RETURN_NUM(UINT32_MAX == index ? -1 : (int)index);
}

DEF_PRIMITIVE(string_indexOf2) {
	if (!validateString(vm, args[1], "Argument"))
		return false;

	ObjString* string = AS_STRING(args[0]);
	ObjString* search = AS_STRING(args[1]);
	u32_t start = validateIndex(vm, args[2], string->length, "Start");
	if (UINT32_MAX == start)
		return false;

	u32_t index = wrenStringFind(string, search, start);
	RETURN_NUM(UINT32_MAX == index ? -1 : (int)index);
}

DEF_PRIMITIVE(string_iterate) {
	ObjString* string = AS_STRING(args[0]);
	if (IS_NULL(args[1])) {
		if (0 == string->length)
			RETURN_FALSE;
		RETURN_NUM(0);
	}

	if (!validateInt(vm, args[1], "Iterator"))
		return false;

	if (AS_NUM(args[1]) < 0)
		RETURN_FALSE;
	u32_t index = (u32_t)AS_NUM(args[1]);

	do {
		++index;
		if (index >= string->length)
			RETURN_FALSE;
	} while (0x80 == (string->value[index] & 0xc0));

	RETURN_NUM(index);
}

DEF_PRIMITIVE(string_iterateByte) {
	ObjString* string = AS_STRING(args[0]);
	if (IS_NULL(args[1])) {
		if (0 == string->length)
			RETURN_FALSE;
		RETURN_NUM(0);
	}

	if (!validateInt(vm, args[1], "Iterator"))
		return false;

	if (AS_NUM(args[1]) < 0)
		RETURN_FALSE;

	u32_t index = (u32_t)AS_NUM(args[1]);
	++index;
	if (index >= string->length)
		RETURN_FALSE;

	RETURN_NUM(index);
}

DEF_PRIMITIVE(string_iteratorValue) {
	ObjString* string = AS_STRING(args[0]);
	u32_t index = validateIndex(vm, args[1], string->length, "Iterator");
	if (UINT32_MAX == index)
		return false;

	RETURN_VAL(wrenStringCodePointAt(vm, string, index));
}

DEF_PRIMITIVE(string_startsWith) {
	if (!validateString(vm, args[1], "Argument"))
		return false;

	ObjString* string = AS_STRING(args[0]);
	ObjString* search = AS_STRING(args[1]);
	if (search->length > string->length)
		RETURN_FALSE;

	RETURN_BOOL(0 == memcmp(string->value, search->value, search->length));
}

DEF_PRIMITIVE(string_plus) {
	if (!validateString(vm, args[1], "Right operand"))
		return false;
	RETURN_VAL(wrenStringFormat(vm, "@@", args[0], args[1]));
}

DEF_PRIMITIVE(string_subscript) {
	ObjString* string = AS_STRING(args[0]);
	if (IS_NUM(args[1])) {
		int index = validateIndex(vm, args[1], string->length, "Subscript");
		if (-1 == index)
			return false;

		RETURN_VAL(wrenStringCodePointAt(vm, string, index));
	}

	if (!IS_RANGE(args[1]))
		RETURN_ERROR("Subscript must be a number or a range.");

	int step;
	u32_t count = string->length;
	int start = calculateRange(vm, AS_RANGE(args[1]), &count, &step);
	if (-1 == start)
		return false;

	RETURN_VAL(wrenNewStringFromRange(vm, string, start, count, step));
}

DEF_PRIMITIVE(string_toString) {
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(system_clock) {
	RETURN_NUM((double)clock() / CLOCKS_PER_SEC);
}

DEF_PRIMITIVE(system_gc) {
	wrenCollectGrabage(vm);
	RETURN_NULL;
}

DEF_PRIMITIVE(system_writeString) {
	if (NULL != vm->config.writeFn)
		vm->config.writeFn(vm, AS_CSTRING(args[1]));

	RETURN_VAL(args[1]);
}

void wrenInitializeCore(WrenVM* vm) {
	ObjModule* coreModule = wrenNewModule(vm, NULL);

	WREN_PUSH_ROOT(vm, coreModule);
	wrenMapSet(vm, vm->modules, NULL_VAL, OBJ_VAL(coreModule));
	WREN_POP_ROOT(vm);

	vm->objectClass = defineClass(vm, coreModule, "Object");
	PRIMITIVE(vm->objectClass, "!", object_not);
	PRIMITIVE(vm->objectClass, "==(_)", object_eqeq);
	PRIMITIVE(vm->objectClass, "!=(_)", object_bangeq);
	PRIMITIVE(vm->objectClass, "is(_)", object_is);
	PRIMITIVE(vm->objectClass, "toString", object_toString);
	PRIMITIVE(vm->objectClass, "type", object_type);

	vm->classClass = defineClass(vm, coreModule, "Class");
	wrenBindSuperclass(vm, vm->classClass, vm->objectClass);
	PRIMITIVE(vm->classClass, "name", class_name);
	PRIMITIVE(vm->classClass, "supertype", class_supertype);
	PRIMITIVE(vm->classClass, "toString", class_toString);

	ObjClass* objectMetaClass = defineClass(vm, coreModule, "Object metaclass");
	vm->objectClass->obj.classObj = objectMetaClass;
	objectMetaClass->obj.classObj = vm->classClass;
	vm->classClass->obj.classObj = vm->classClass;

	wrenBindSuperclass(vm, objectMetaClass, vm->classClass);
	PRIMITIVE(objectMetaClass, "same(_,_)", object_same);

	wrenInterpret(vm, NULL, coreModuleSource);

	vm->boolClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Bool"));
	PRIMITIVE(vm->boolClass, "toString", bool_toString);
	PRIMITIVE(vm->boolClass, "!", bool_not);

	vm->fiberClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Fiber"));
	PRIMITIVE(vm->fiberClass->obj.classObj, "new(_)", fiber_new);
	PRIMITIVE(vm->fiberClass->obj.classObj, "abort(_)", fiber_abort);
	PRIMITIVE(vm->fiberClass->obj.classObj, "current", fiber_current);
	PRIMITIVE(vm->fiberClass->obj.classObj, "suspend()", fiber_suspend);
	PRIMITIVE(vm->fiberClass->obj.classObj, "yield()", fiber_yield);
	PRIMITIVE(vm->fiberClass->obj.classObj, "yield(_)", fiber_yield1);
	PRIMITIVE(vm->fiberClass, "call()", fiber_call);
	PRIMITIVE(vm->fiberClass, "call(_)", fiber_call1);
	PRIMITIVE(vm->fiberClass, "error", fiber_error);
	PRIMITIVE(vm->fiberClass, "isDone", fiber_isDone);
	PRIMITIVE(vm->fiberClass, "transfer()", fiber_transfer);
	PRIMITIVE(vm->fiberClass, "transfer(_)", fiber_transfer1);
	PRIMITIVE(vm->fiberClass, "transferError(_)", fiber_transferError);
	PRIMITIVE(vm->fiberClass, "try()", fiber_try);

	vm->fnClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Fn"));
	PRIMITIVE(vm->fnClass->obj.classObj, "new(_)", fn_new);

	PRIMITIVE(vm->fnClass, "arity", fn_arity);
	PRIMITIVE(vm->fnClass, "call()", fn_call0);
	PRIMITIVE(vm->fnClass, "call(_)", fn_call1);
	PRIMITIVE(vm->fnClass, "call(_,_)", fn_call2);
	PRIMITIVE(vm->fnClass, "call(_,_,_)", fn_call3);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_)", fn_call4);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_)", fn_call5);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_)", fn_call6);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_)", fn_call7);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_)", fn_call8);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_)", fn_call9);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_)", fn_call10);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_)", fn_call11);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_)", fn_call12);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call13);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call14);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call15);
	PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call16);
	PRIMITIVE(vm->fnClass, "toString", fn_toString);

	vm->nullClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Null"));
	PRIMITIVE(vm->nullClass, "!", null_not);
	PRIMITIVE(vm->nullClass, "toString", null_toString);

	vm->numClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Num"));
	PRIMITIVE(vm->numClass->obj.classObj, "fromString(_)", num_fromString);
	PRIMITIVE(vm->numClass->obj.classObj, "pi", num_pi);
	PRIMITIVE(vm->numClass->obj.classObj, "largest", num_largest);
	PRIMITIVE(vm->numClass->obj.classObj, "smallest", num_smallest);
	PRIMITIVE(vm->numClass, "-(_)", num_minus);
	PRIMITIVE(vm->numClass, "+(_)", num_plus);
	PRIMITIVE(vm->numClass, "*(_)", num_multiply);
	PRIMITIVE(vm->numClass, "/(_)", num_divide);
	PRIMITIVE(vm->numClass, "<(_)", num_lt);
	PRIMITIVE(vm->numClass, ">(_)", num_gt);
	PRIMITIVE(vm->numClass, "<=(_)", num_lte);
	PRIMITIVE(vm->numClass, ">=(_)", num_gte);
	PRIMITIVE(vm->numClass, "&(_)", num_bitwiseAnd);
	PRIMITIVE(vm->numClass, "|(_)", num_bitwiseOr);
	PRIMITIVE(vm->numClass, "^(_)", num_bitwiseXor);
	PRIMITIVE(vm->numClass, "<<(_)", num_bitwiseLeftShift);
	PRIMITIVE(vm->numClass, ">>(_)", num_bitwiseRightShift);
	PRIMITIVE(vm->numClass, "abs", num_abs);
	PRIMITIVE(vm->numClass, "acos", num_acos);
	PRIMITIVE(vm->numClass, "asin", num_asin);
	PRIMITIVE(vm->numClass, "atan", num_atan);
	PRIMITIVE(vm->numClass, "ceil", num_ceil);
	PRIMITIVE(vm->numClass, "cos", num_cos);
	PRIMITIVE(vm->numClass, "floor", num_floor);
	PRIMITIVE(vm->numClass, "-", num_negate);
	PRIMITIVE(vm->numClass, "round", num_round);
	PRIMITIVE(vm->numClass, "sin", num_sin);
	PRIMITIVE(vm->numClass, "sqrt", num_sqrt);
	PRIMITIVE(vm->numClass, "tan", num_tan);
	PRIMITIVE(vm->numClass, "log", num_log);
	PRIMITIVE(vm->numClass, "%(_)", num_mod);
	PRIMITIVE(vm->numClass, "~", num_bitwiseNot);
	PRIMITIVE(vm->numClass, "..(_)", num_dotDot);
	PRIMITIVE(vm->numClass, "...(_)", num_dotDotDot);
	PRIMITIVE(vm->numClass, "atan(_)", num_atan2);
	PRIMITIVE(vm->numClass, "fraction", num_fraction);
	PRIMITIVE(vm->numClass, "isInfinity", num_isInfinity);
	PRIMITIVE(vm->numClass, "isInteger", num_isInteger);
	PRIMITIVE(vm->numClass, "isNan", num_isNan);
	PRIMITIVE(vm->numClass, "sign", num_sign);
	PRIMITIVE(vm->numClass, "toString", num_toString);
	PRIMITIVE(vm->numClass, "truncate", num_truncate);
	PRIMITIVE(vm->numClass, "==(_)", num_eqeq);
	PRIMITIVE(vm->numClass, "!=(_)", num_bangeq);

	vm->stringClass = AS_CLASS(wrenFindVariable(vm, coreModule, "String"));
	PRIMITIVE(vm->stringClass->obj.classObj, "fromCodePoint(_)", string_fromCodePoint);
	PRIMITIVE(vm->stringClass->obj.classObj, "fromByte(_)", string_fromByte);
	PRIMITIVE(vm->stringClass, "+(_)", string_plus);
	PRIMITIVE(vm->stringClass, "[_]", string_subscript);
	PRIMITIVE(vm->stringClass, "byteAt_(_)", string_byteAt);
	PRIMITIVE(vm->stringClass, "byteCount_", string_byteCount);
	PRIMITIVE(vm->stringClass, "codePointAt_(_)", string_codePointAt);
	PRIMITIVE(vm->stringClass, "contains(_)", string_contains);
	PRIMITIVE(vm->stringClass, "endsWith(_)", string_endsWith);
	PRIMITIVE(vm->stringClass, "indexOf(_)", string_indexOf1);
	PRIMITIVE(vm->stringClass, "indexOf(_,_)", string_indexOf2);
	PRIMITIVE(vm->stringClass, "iterate(_)", string_iterate);
	PRIMITIVE(vm->stringClass, "iterateByte_(_)", string_iterateByte);
	PRIMITIVE(vm->stringClass, "iteratorValue(_)", string_iteratorValue);
	PRIMITIVE(vm->stringClass, "startsWith(_)", string_startsWith);
	PRIMITIVE(vm->stringClass, "toString", string_toString);

	vm->listClass = AS_CLASS(wrenFindVariable(vm, coreModule, "List"));
	PRIMITIVE(vm->listClass->obj.classObj, "filled(_,_)", list_filled);
	PRIMITIVE(vm->listClass->obj.classObj, "new()", list_new);
	PRIMITIVE(vm->listClass, "[_]", list_subscript);
	PRIMITIVE(vm->listClass, "[_]=(_)", list_subscriptSetter);
	PRIMITIVE(vm->listClass, "add(_)", list_add);
	PRIMITIVE(vm->listClass, "addCore_(_)", list_addCore);
	PRIMITIVE(vm->listClass, "clear()", list_clear);
	PRIMITIVE(vm->listClass, "count", list_count);
	PRIMITIVE(vm->listClass, "insert(_,_)", list_insert);
	PRIMITIVE(vm->listClass, "iterate(_)", list_iterate);
	PRIMITIVE(vm->listClass, "iteratorValue(_)", list_iteratorValue);
	PRIMITIVE(vm->listClass, "removeAt(_)", list_removeAt);

	vm->mapClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Map"));
	PRIMITIVE(vm->mapClass->obj.classObj, "new()", map_new);
	PRIMITIVE(vm->mapClass, "[_]", map_subscript);
	PRIMITIVE(vm->mapClass, "[_]=(_)", map_subscriptSetter);
	PRIMITIVE(vm->mapClass, "addCore_(_)", map_addCore);
	PRIMITIVE(vm->mapClass, "clear()", map_clear);
	PRIMITIVE(vm->mapClass, "containsKey(_)", map_containsKey);
	PRIMITIVE(vm->mapClass, "count", map_count);
	PRIMITIVE(vm->mapClass, "remove(_)", map_remove);
	PRIMITIVE(vm->mapClass, "iterate(_)", map_iterate);
	PRIMITIVE(vm->mapClass, "keyIteratorValue_(_)", map_keyIteratorValue);
	PRIMITIVE(vm->mapClass, "valueIteratorValue_(_)", map_valueIteratorValue);

	vm->rangeClass = AS_CLASS(wrenFindVariable(vm, coreModule, "Range"));
	PRIMITIVE(vm->rangeClass, "from", range_from);
	PRIMITIVE(vm->rangeClass, "to", range_to);
	PRIMITIVE(vm->rangeClass, "min", range_min);
	PRIMITIVE(vm->rangeClass, "max", range_max);
	PRIMITIVE(vm->rangeClass, "isInclusive", range_isInclusive);
	PRIMITIVE(vm->rangeClass, "iterate(_)", range_iterate);
	PRIMITIVE(vm->rangeClass, "iteratorValue(_)", range_iteratorValue);
	PRIMITIVE(vm->rangeClass, "toString", range_toString);

	ObjClass* systemClass = AS_CLASS(wrenFindVariable(vm, coreModule, "System"));
	PRIMITIVE(systemClass->obj.classObj, "clock", system_clock);
	PRIMITIVE(systemClass->obj.classObj, "gc()", system_gc);
	PRIMITIVE(systemClass->obj.classObj, "writeString_(_)", system_writeString);

	for (Obj* obj = vm->first; NULL != obj; obj = obj->next) {
		if (OBJ_STRING == obj->type)
			obj->classObj = vm->stringClass;
	}
}
