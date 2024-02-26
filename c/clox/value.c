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
#include "memory.h"
#include "object.h"
#include "value.h"

bool valuesEqual(Value a, Value b) {
	if (a.type != b.type)
		return false;
	switch (a.type) {
	case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
	case VAL_NIL: return true;
	case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
	case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);
	default: break;
	}
	return false;
}

void printValue(Value value) {
	switch (value.type) {
	case VAL_BOOL: fprintf(stdout, AS_BOOL(value) ? "true" : "false"); break;
	case VAL_NIL: fprintf(stdout, "nil"); break;
	case VAL_NUMBER: fprintf(stdout, "%g", AS_NUMBER(value)); break;
	case VAL_OBJ: printObject(value); break;
	}
}

void initValueArray(ValueArray* array) {
	array->count = 0;
	array->capacity = 0 ;
	array->values = NULL;
}

void freeValueArray(ValueArray* array) {
	FREE_ARRAY(Value, array->values, array->capacity);
	initValueArray(array);
}

void writeValueArray(ValueArray* array, Value value) {
	if (array->capacity < array->count + 1) {
		int oldCapacity = array->capacity;
		array->capacity = GROW_CAPACITY(oldCapacity);
		array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
	}

	array->values[array->count++] = value;
}
