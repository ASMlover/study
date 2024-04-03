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
#include "debug.h"

static void dumpObject(Obj* obj) {
	switch (obj->type) {
	case OBJ_CLASS:    fprintf(stdout, "[class %s %p]", ((ObjClass*)obj)->name->value, obj); break;
	case OBJ_CLOSURE:  fprintf(stdout, "[closure %p]", obj); break;
	case OBJ_FIBER:    fprintf(stdout, "[fiber %p]", obj); break;
	case OBJ_FN:       fprintf(stdout, "[fn %p]", obj); break;
	case OBJ_FOREIGN:  fprintf(stdout, "[foreign %p]", obj); break;
	case OBJ_INSTANCE: fprintf(stdout, "[instance %p]", obj); break;
	case OBJ_LIST:     fprintf(stdout, "[list %p]", obj); break;
	case OBJ_MAP:      fprintf(stdout, "[map %p]", obj); break;
	case OBJ_MODULE:   fprintf(stdout, "[module %p]", obj); break;
	case OBJ_RANGE:    fprintf(stdout, "[range %p]", obj); break;
	case OBJ_STRING:   fprintf(stdout, "%s", ((ObjString*)obj)->value); break;
	case OBJ_UPVALUE:  fprintf(stdout, "[upvalue %p]", obj); break;
	default:           fprintf(stdout, "[unknown object %d]", obj->type); break;
	}
}

static int dumpInstruction(WrenVM* vm, ObjFn* fn, int i, int* lastLine) {
	int start = i;
	u8_t* bytecode = fn->code.data;
	Code code = (Code)bytecode[i];

	int lineno = fn->debug->sourceLines.data[i];
	if (NULL == lastLine || *lastLine != lineno) {
		fprintf(stdout, "%4d: ", lineno);

		if (NULL != lastLine)
			*lastLine = lineno;
	}
	else {
		fprintf(stdout, "     ");
	}
	fprintf(stdout, " %04d  ", i++);

#define READ_BYTE()                         (bytecode[i++])
#define READ_SHORT()                        (i += 2, (bytecode[i - 2] << 8) | bytecode[i - 1])
#define BYTE_INSTRUCTION(name)              fprintf(stdout, "%-16s %5d\n", #name, READ_BYTE()); break
#define OPCODE_DISPLAY(name)                fprintf(stdout, #name "\n"); break
#define VALUE_DISPLAY(name)\
	{\
		int slot = READ_SHORT();\
		fprintf(stdout, "%-16s %5d `%s`\n", #name, slot, fn->module->variableNames.data[slot]->value);\
	} break
#define JUMP_DISPLAY(name, op)\
	{\
		int offset = READ_SHORT();\
		fprintf(stdout, "%-16s %5d to %d\n", #name, (offset), i op offset);\
	} break

	switch (code) {
	case CODE_CONSTANT:
		{
			int constant = READ_SHORT();
			fprintf(stdout, "%-16s %5d `", "CONSTANT", constant);
			wrenDumpValue(fn->constants.data[constant]);
			fprintf(stdout, "`\n");
		} break;
	case CODE_NULL:  OPCODE_DISPLAY(NULL);
	case CODE_FALSE: OPCODE_DISPLAY(FALSE);
	case CODE_TRUE:  OPCODE_DISPLAY(TRUE);

	case CODE_LOAD_LOCAL_0: OPCODE_DISPLAY(LOAD_LOCAL_0);
	case CODE_LOAD_LOCAL_1: OPCODE_DISPLAY(LOAD_LOCAL_1);
	case CODE_LOAD_LOCAL_2: OPCODE_DISPLAY(LOAD_LOCAL_2);
	case CODE_LOAD_LOCAL_3: OPCODE_DISPLAY(LOAD_LOCAL_3);
	case CODE_LOAD_LOCAL_4: OPCODE_DISPLAY(LOAD_LOCAL_4);
	case CODE_LOAD_LOCAL_5: OPCODE_DISPLAY(LOAD_LOCAL_5);
	case CODE_LOAD_LOCAL_6: OPCODE_DISPLAY(LOAD_LOCAL_6);
	case CODE_LOAD_LOCAL_7: OPCODE_DISPLAY(LOAD_LOCAL_7);
	case CODE_LOAD_LOCAL_8: OPCODE_DISPLAY(LOAD_LOCAL_8);

	case CODE_LOAD_LOCAL:    BYTE_INSTRUCTION(LOAD_LOCAL);
	case CODE_STORE_LOCAL:   BYTE_INSTRUCTION(STORE_LOCAL);
	case CODE_LOAD_UPVALUE:  BYTE_INSTRUCTION(LOAD_UPVALUE);
	case CODE_STORE_UPVALUE: BYTE_INSTRUCTION(STORE_UPVALUE);

	case CODE_LOAD_MODULE_VAR:  VALUE_DISPLAY(LOAD_MODULE_VAR);
	case CODE_STORE_MODULE_VAR: VALUE_DISPLAY(STORE_MODULE_VAR);

	case CODE_LOAD_FIELD_THIS:  BYTE_INSTRUCTION(LOAD_FIELD_THIS);
	case CODE_STORE_FIELD_THIS: BYTE_INSTRUCTION(STORE_FIELD_THIS);
	case CODE_LOAD_FIELD:       BYTE_INSTRUCTION(LOAD_FIELD);
	case CODE_STORE_FIELD:      BYTE_INSTRUCTION(STORE_FIELD);
	case CODE_POP:              OPCODE_DISPLAY(POP);

	case CODE_CALL_0:
	case CODE_CALL_1:
	case CODE_CALL_2:
	case CODE_CALL_3:
	case CODE_CALL_4:
	case CODE_CALL_5:
	case CODE_CALL_6:
	case CODE_CALL_7:
	case CODE_CALL_8:
	case CODE_CALL_9:
	case CODE_CALL_10:
	case CODE_CALL_11:
	case CODE_CALL_12:
	case CODE_CALL_13:
	case CODE_CALL_14:
	case CODE_CALL_15:
	case CODE_CALL_16:
		{
			int numArgs = bytecode[i - 1] - CODE_CALL_0;
			int symbol = READ_SHORT();
			fprintf(stdout, "CALL_%-11d %5d `%s`\n", numArgs, symbol, vm->methodNames.data[symbol]->value);
		} break;

	case CODE_SUPER_0:
	case CODE_SUPER_1:
	case CODE_SUPER_2:
	case CODE_SUPER_3:
	case CODE_SUPER_4:
	case CODE_SUPER_5:
	case CODE_SUPER_6:
	case CODE_SUPER_7:
	case CODE_SUPER_8:
	case CODE_SUPER_9:
	case CODE_SUPER_10:
	case CODE_SUPER_11:
	case CODE_SUPER_12:
	case CODE_SUPER_13:
	case CODE_SUPER_14:
	case CODE_SUPER_15:
	case CODE_SUPER_16:
		{
			int numArgs = bytecode[i - 1] - CODE_SUPER_0;
			int symbol = READ_SHORT();
			int superclass = READ_SHORT();
			fprintf(stdout, "SUPER_%-10d %5d `%s` %5d\n",
					numArgs, symbol, vm->methodNames.data[symbol]->value, superclass);
		} break;

	case CODE_JUMP:    JUMP_DISPLAY(JUMP, +);
	case CODE_LOOP:    JUMP_DISPLAY(LOOP, -);
	case CODE_JUMP_IF: JUMP_DISPLAY(JUMP_IF, +);
	case CODE_AND:     JUMP_DISPLAY(AND, +);
	case CODE_OR:      JUMP_DISPLAY(OR, +);

	case CODE_CLOSE_UPVALUE: OPCODE_DISPLAY(CLOSE_UPVALUE);
	case CODE_RETURN:        OPCODE_DISPLAY(RETURN);

	case CODE_CLOSURE:
		{
		} break;

	default: fprintf(stdout, "UNKNOWN! [%d]\n", bytecode[i - 1]); break;
	}
#undef JUMP_DISPLAY
#undef VALUE_DISPLAY
#undef OPCODE_DISPLAY
#undef BYTE_INSTRUCTION
#undef READ_SHORT
#undef READ_BYTE

	if (code == CODE_END)
		return -1;
	return i - start;
}

void wrenDebugPrintStackTrace(WrenVM* vm) {
	if (NULL == vm->config.errorFn)
		return;

	ObjFiber* fiber = vm->fiber;
	if (IS_STRING(fiber->error))
		vm->config.errorFn(vm, WREN_ERROR_RUNTIME, NULL, -1, AS_CSTRING(fiber->error));
	else
		vm->config.errorFn(vm, WREN_ERROR_RUNTIME, NULL, -1, "[error object]");

	for (int i = fiber->numFrames - 1; i >= 0; --i) {
		CallFrame* frame = &fiber->frames[i];
		ObjFn* fn = frame->closure->fn;

		if (NULL == fn->module)
			continue;
		if (NULL == fn->module->name)
			continue;

		int lineno = fn->debug->sourceLines.data[frame->ip - fn->code.data - 1];
		vm->config.errorFn(vm, WREN_ERROR_STACK_TRACE, fn->module->name->value, lineno, fn->debug->name);
	}
}

void wrenDumpValue(Value value) {
	switch (value.type) {
	case VAL_FALSE:     fprintf(stdout, "false"); break;
	case VAL_NULL:      fprintf(stdout, "null"); break;
	case VAL_NUM:       fprintf(stdout, "%.14g", AS_NUM(value)); break;
	case VAL_TRUE:      fprintf(stdout, "true"); break;
	case VAL_OBJ:       dumpObject(AS_OBJ(value)); break;
	case VAL_UNDEFINED: UNREACHABLE();
	}
}

int wrenDumpInstruction(WrenVM* vm, ObjFn* fn, int i) { return 0; }
void wrenDumpCode(WrenVM* vm, ObjFn* fn) {}
void wrenDumpStack(ObjFiber* fiber) {}
