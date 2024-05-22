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
#ifndef WREN_PRIMITIVE_H
#define WREN_PRIMITIVE_H

#include "vm.h"

#define PRIMITIVE(cls, name, function)\
	{\
		int symbol = wrenSymbolTableEnsure(vm, &vm->methodNames, name, strlen(name));\
		Method method;\
		method.type = METHOD_PRIMITIVE;\
		method.as.primitive = prim_##function;\
		wrenBindMethod(vm, cls, symbol, method);\
	}

#define DEF_PRIMITIVE(name)\
	static bool prim_##name(WrenVM* vm, Value* args)

#define RETURN_VAL(value)                   do { args[0] = value; return true; } while (false)
#define RETURN_OBJ(obj)                     RETURN_VAL(OBJ_VAL(obj))
#define RETURN_BOOL(value)                  RETURN_VAL(BOOL_VAL(value))
#define RETURN_FALSE                        RETURN_VAL(FALSE_VAL)
#define RETURN_NULL                         RETURN_VAL(NULL_VAL)
#define RETURN_NUM(value)                   RETURN_VAL(NUM_VAL(value))
#define RETURN_TRUE                         RETURN_VAL(TRUE_VAL)

#define RETURN_ERROR(msg)\
	do {\
		vm->fiber->error = wrenNewStringLength(vm, msg, sizeof(msg) - 1);\
		return false;\
	} while (false)

#define RETURN_ERROR_FMT(msg, arg)\
	do {\
		vm->fiber->error = wrenStringFormat(vm, msg, arg);\
		return false;\
	} while (false)

bool validateFn(WrenVM* vm, Value arg, const char* argName);
bool validateNum(WrenVM* vm, Value arg, const char* argName);
bool validateIntValue(WrenVM* vm, double value, const char* argName);
bool validateInt(WrenVM* vm, Value arg, const char* argName);
bool validateKey(WrenVM* vm, Value arg);
u32_t validateIndex(WrenVM* vm, Value arg, u32_t count, const char* argName);
bool validateString(WrenVM* vm, Value arg, const char* argName);
u32_t calculateRange(WrenVM* vm, ObjRange* range, u32_t* length, int* step);

#endif
