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
#include <math.h>
#include "primitive.h"

static u32_t validateIndexValue(WrenVM* vm, u32_t count, double value, const char* argName) {
	if (!validateIntValue(vm, value, argName))
		return UINT32_MAX;

	if (value < 0)
		value = count + value;
	if (value >= 0 && value < count)
		return (u32_t)value;

	vm->fiber->error = wrenStringFormat(vm, "$ out of bounds.", argName);
	return UINT32_MAX;
}

bool validateFn(WrenVM* vm, Value arg, const char* argName) {
	if (IS_CLOSURE(arg))
		return true;
	RETURN_ERROR_FMT("$ must be a function.", argName);
}

bool validateNum(WrenVM* vm, Value arg, const char* argName) {
	if (IS_NUM(arg))
		return true;
	RETURN_ERROR_FMT("$ must be a number.", argName);
}

bool validateIntValue(WrenVM* vm, double value, const char* argName) {
	if (trunc(value) == value)
		return true;
	RETURN_ERROR_FMT("$ must be an integer.", argName);
}

bool validateInt(WrenVM* vm, Value arg, const char* argName) {
	if (!validateNum(vm, arg, argName))
		return false;
	return validateIntValue(vm, AS_NUM(arg), argName);
}

bool validateKey(WrenVM* vm, Value arg) {
	if (IS_BOOL(arg) || IS_CLASS(arg) || IS_NULL(arg) || IS_NUM(arg) || IS_RANGE(arg) || IS_STRING(arg))
		return true;
	RETURN_ERROR("Key must be a value type.");
}

u32_t validateIndex(WrenVM* vm, Value arg, u32_t count, const char* argName) {
	if (!validateNum(vm, arg, argName))
		return UINT32_MAX;
	return validateIndexValue(vm, count, AS_NUM(arg), argName);
}

bool validateString(WrenVM* vm, Value arg, const char* argName) {
	if (IS_STRING(arg))
		return true;
	RETURN_ERROR_FMT("$ must be a string.", argName);
}

u32_t calculateRange(WrenVM* vm, ObjRange* range, u32_t* length, int* step) {
	*step = 0;

	if (range->from == *length && range->to == (range->isInclusive ? -1.0 : (double)*length)) {
		*length = 0;
		return 0;
	}

	u32_t from = validateIndexValue(vm, *length, range->from, "Range start");
	if (UINT32_MAX == from)
		return UINT32_MAX;

	double value = range->to;
	if (!validateIntValue(vm, value, "Range end"))
		return UINT32_MAX;

	if (value < 0)
		value = *length + value;

	if (!range->isInclusive) {
		if (value == from) {
			*length = 0;
			return from;
		}

		value += value >= from ? -1 : 1;
	}

	if (value < 0 || value >= *length) {
		vm->fiber->error = CONST_STRING(vm, "Range end out of bounds.");
		return UINT32_MAX;
	}

	u32_t to = (u32_t)value;
	*length = (u32_t)fabs((int)(from - to)) + 1;
	*step = from < to ? 1 : -1;
	return from;
}
