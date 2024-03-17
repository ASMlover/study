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
#include "vm.h"

void* wrenReallocate(WrenVM* vm, void* memory, sz_t oldSize, sz_t newSize) { return NULL; }
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
