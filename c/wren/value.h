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
#ifndef WREN_VALUE_H
#define WREN_VALUE_H

#include <string.h>

#include "common.h"
#include "utils.h"

#define AS_CLASS(value)                 ((ObjClass*)AS_OBJ(value))              // ObjClass*
#define AS_CLOSURE(value)               ((ObjClosure*)AS_OBJ(value))            // ObjClosure*
#define AS_FIBER(value)                 ((ObjFiber*)AS_OBJ(value))              // ObjFiber*
#define AS_FN(value)                    ((ObjFn*)AS_OBJ(value))                 // ObjFn*
#define AS_FOREIGN(value)               ((ObjForeign*)AS_OBJ(value))            // ObjForeign*
#define AS_INSTANCE(value)              ((ObjInstance*)AS_OBJ(value))           // ObjInstance*
#define AS_LIST(value)                  ((ObjList*)AS_OBJ(value))               // ObjList*
#define AS_MAP(value)                   ((ObjMap*)AS_OBJ(value))                // ObjMap*
#define AS_MODULE(value)                ((ObjModule*)AS_OBJ(value))             // ObjModule*
#define AS_NUM(value)                   (wrenValueToNum(value))                 // double
#define AS_RANGE(value)                 ((ObjRange*)AS_OBJ(value))              // ObjRange*
#define AS_STRING(value)                ((ObjString*)AS_OBJ(value))             // ObjString*
#define AS_CSTRING(value)               (AS_STRING(value)->value)               // const char*
#define AS_UPVALUE(value)               ((ObjUpvalue*)AS_OBJ(value))            // ObjUpvalue*

#define BOOL_VAL(boolean)               ((boolean) ? TRUE_VAL : FALSE_VAL)      // boolean
#define NUM_VAL(num)                    (wrenNumToValue(num))                   // double
#define OBJ_VAL(obj)                    (wrenObjectToValue((Obj*)(obj)))        // Any Objxxx*

#define IS_BOOL(value)                  (wrenIsBool(value))                     // Bool
#define IS_CLASS(value)                 (wrenIsObjType(value, OBJ_CLASS))       // ObjClass
#define IS_CLOSURE(vlaue)               (wrenIsObjType(value, OBJ_CLOSURE))     // ObjClosure
#define IS_FIBER(value)                 (wrenIsObjType(value, OBJ_FIBER))       // ObjFiber
#define IS_FN(value)                    (wrenIsObjType(value, OBJ_FN))
#define IS_FOREIGN(value)               (wrenIsObjType(value, OBJ_FOREIGN))     // ObjForeign
#define IS_INSTANCE(value)              (wrenIsObjType(value, OBJ_INSTANCE))    // ObjInstance
#define IS_LIST(value)                  (wrenIsObjType(value, OBJ_LIST))        // ObjList
#define IS_MAP(value)                   (wrenIsObjType(value, OBJ_MAP))         // ObjMap
#define IS_MODULE(value)                (wrenIsObjType(value, OBJ_MODULE))      // ObjModule
#define IS_RANGE(value)                 (wrenIsObjType(value, OBJ_RANGE))       // ObjRange
#define IS_STRING(value)                (wrenIsObjType(value, OBJ_STRING))      // ObjString
#define IS_UPVALUE(value)               (wrenIsObjType(value, OBJ_UPVALUE))     // ObjUpvalue

#define CONST_STRING(vm, text)          wrenNewStringLength((vm), (text), sizeof(text) - 1)

typedef enum {
	OBJ_CLASS,
	OBJ_CLOSURE,
	OBJ_FIBER,
	OBJ_FN,
	OBJ_FOREIGN,
	OBJ_INSTANCE,
	OBJ_LIST,
	OBJ_MAP,
	OBJ_MODULE,
	OBJ_RANGE,
	OBJ_STRING,
	OBJ_UPVALUE,
} ObjType;

typedef struct ObjClass ObjClass;
typedef struct Obj Obj;

struct Obj {
	ObjType type;
	bool isDark;
	ObjClass* classObj;
	struct Obj* next;
};

typedef enum {
	VAL_FALSE,
	VAL_NULL,
	VAL_NUM,
	VAL_TRUE,
	VAL_UNDEFINED,
	VAL_OBJ,
} ValueType;

typedef struct {
	ValueType type;
	union {
		double num;
		Obj* obj;
	} as;
} Value;
DECLARE_BUFFER(Value, Value);

struct ObjString {
	Obj obj;

	u32_t length;
	u32_t hash;
	char value[WREN_FLEXIBLE_ARRAY];
};

typedef struct ObjUpvalue {
	Obj obj;

	Value* value;
	Value closed;
	struct ObjUpvalue* next;
} ObjUpvalue;

typedef bool (*Primitive)(WrenVM* vm, Value* args);

typedef struct {
	char* name;
	IntBuffer sourceLines;
} FnDebug;

typedef struct {
	Obj obj;

	ValueBuffer variables;
	SymbolTable variableNames;
	ObjString* name;
} ObjModule;

typedef struct {
	Obj obj;

	ByteBuffer code;
	ValueBuffer constants;
	ObjModule* module;
	int maxSlots;
	int numUpvalues;
	int arity;
	FnDebug* debug;
} ObjFn;

typedef struct {
	Obj obj;

	ObjFn* fn;
	ObjUpvalue* upvalues[WREN_FLEXIBLE_ARRAY];
} ObjClosure;

typedef struct {
	u8_t* ip;
	ObjClosure* closure;
	Value* stackStart;
} CallFrame;

typedef enum {
	FIBER_TRY,
	FIBER_ROOT,
	FIBER_OTHER,
} FiberState;

typedef struct ObjFiber {
	Obj obj;

	Value* stack;
	Value* stackTop;
	int stackCapacity;
	CallFrame* frames;
	int numFrames;
	int frameCapacity;
	ObjUpvalue* openUpvalues;
	struct ObjFiber* caller;
	Value error;
	FiberState state;
} ObjFiber;

typedef enum {
	METHOD_PRIMITIVE,
	METHOD_FOREIGN,
	METHOD_BLOCK,
	METHOD_NONE,
} MethodType;

typedef struct {
	MethodType type;
	union {
		Primitive primitive;
		WrenForeignMethodFn foreign;
		ObjClosure* closure;
	} as;
} Method;
DECLARE_BUFFER(Method, Method);

struct ObjClass {
	Obj obj;

	ObjClass* superclass;
	int numFields;
	MethodBuffer methods;
	ObjString* name;
};

typedef struct {
	Obj obj;

	u8_t data[WREN_FLEXIBLE_ARRAY];
} ObjForeign;

typedef struct {
	Obj obj;

	Value fields[WREN_FLEXIBLE_ARRAY];
} ObjInstance;

typedef struct {
	Obj obj;

	ValueBuffer elements;
} ObjList;

typedef struct {
	Value key;
	Value value;
} MapEntry;

typedef struct {
	Obj obj;

	u32_t count;
	u32_t capacity;
	MapEntry* entries;
} ObjMap;

typedef struct {
	Obj obj;

	double from;
	double to;
	bool isInclusive;
} ObjRange;

#define AS_BOOL(value)                  ((value).type == VAL_TRUE)
#define AS_OBJ(value)                   ((value).as.obj)

#define IS_OBJ(value)                   ((value).type == VAL_OBJ)
#define IS_FALSE(value)                 ((value).type == VAL_FALSE)
#define IS_NULL(value)                  ((value).type == VAL_NULL)
#define IS_NUM(value)                   ((value).type == VAL_NUM)
#define IS_UNDEFINED(value)             ((value).type == VAL_UNDEFINED)

#define FALSE_VAL                       ((Value){VAL_FALSE, {0}})
#define NULL_VAL                        ((Value){VAL_NULL, {0}})
#define TRUE_VAL                        ((Value){VAL_TRUE, {0}})
#define UNDEFINED_VAL                   ((Value){VAL_UNDEFINED, {0}})

typedef union {
	u64_t bits64;
	u32_t bits32[2];
	double num;
} DoubleBits;

ObjClass* wrenNewSingleClass(WrenVM* vm, int numFields, ObjString* name);
void wrenBindSuperclass(WrenVM* vm, ObjClass* subclass, ObjClass* superclass);
ObjClass* wrenNewClass(WrenVM* vm, ObjClass* superclass, int numFields, ObjString* name);
void wrenBindMethod(WrenVM* vm, ObjClass* classObj, int symbol, Method method);

ObjClosure* wrenNewClosure(WrenVM* vm, ObjFn* fn);
ObjFiber* wrenNewFiber(WrenVM* vm, ObjClosure* closure);

static inline void wrenAppendCallFrame(WrenVM* vm, ObjFiber* fiber, ObjClosure* closure, Value* stackStart) {
	ASSERT(fiber->frameCapacity > fiber->numFrames, "No memory for call frame.");

	CallFrame* frame = &fiber->frames[fiber->numFrames++];
	frame->stackStart = stackStart;
	frame->closure = closure;
	frame->ip = closure->fn->code.data;
}

void wrenEnsureStack(WrenVM* vm, ObjFiber* fiber, int needed);

static inline bool wrenHasError(const ObjFiber* fiber) {
	return !IS_NULL(fiber->error);
}

ObjForeign* wrenNewForeign(WrenVM* vm, ObjClass* classObj, sz_t size);

ObjFn* wrenNewFunction(WrenVM* vm, ObjModule* module, int maxSlots);
void wrenFunctionBindName(WrenVM* vm, ObjFn* fn, const char* name, int length);

Value wrenNewInstance(WrenVM* vm, ObjClass* classObj);

ObjList* wrenNewList(WrenVM* vm, u32_t numElements);
void wrenListInsert(WrenVM* vm, ObjList* list, Value value, u32_t index);
Value wrenListRemoveAt(WrenVM* vm, ObjList* list, u32_t index);

ObjMap* wrenNewMap(WrenVM* vm);
void wrenMapClear(WrenVM* vm, ObjMap* map);
Value wrenMapGet(ObjMap* map, Value key);
void wrenMapSet(WrenVM* vm, ObjMap* map, Value key, Value value);
Value wrenMapRemoveKey(WrenVM* vm, ObjMap* map, Value key);

ObjModule* wrenNewModule(WrenVM* vm, ObjString* name);
Value wrenNewRange(WrenVM* vm, double from, double to, bool isInclusive);

Value wrenNewString(WrenVM* vm, const char* text);
Value wrenNewStringLength(WrenVM* vm, const char* text, sz_t length);
Value wrenNewStringFromRange(WrenVM* vm, ObjString* source, int start, u32_t count, int step);
Value wrenNumToString(WrenVM* vm, double value);
Value wrenStringFormat(WrenVM* vm, const char* format, ...);
Value wrenStringFromCodePoint(WrenVM* vm, int value);
Value wrenStringFromByte(WrenVM* vm, u8_t value);
Value wrenStringCodePointAt(WrenVM* vm, ObjString* string, u32_t index);
u32_t wrenStringFind(ObjString* haystack, ObjString* needle, u32_t startIndex);

static inline bool wrenStringEqualsCString(const ObjString* a, const char* b, sz_t length) {
	return a->length == length && memcmp(a->value, b, length) == 0;
}

ObjUpvalue* wrenNewUpvalue(WrenVM* vm, Value* value);

void wrenGrayObj(WrenVM* vm, Obj* obj);
void wrenGrayValue(WrenVM* vm, Value value);
void wrenGrayBuffer(WrenVM* vm, ValueBuffer* buffer);
void wrenBlackenObjects(WrenVM* vm);
void wrenFreeObj(WrenVM* vm, Obj* obj);

ObjClass* wrenGetClass(WrenVM* vm, Value value);

static inline bool wrenValuesSame(Value a, Value b) {
	if (a.type != b.type)
		return false;
	if (a.type == b.type)
		return a.as.num == b.as.num;
	return a.as.obj == b.as.obj;
}

bool wrenValuesEqual(Value a, Value b);

static inline bool wrenIsBool(Value value) {
	return value.type == VAL_FALSE || value.type == VAL_TRUE;
}

static inline bool wrenIsObjType(Value value, ObjType type) {
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

static inline Value wrenObjectToValue(Obj* obj) {
	Value value;
	value.type = VAL_OBJ;
	value.as.obj = obj;
	return value;
}

static inline double wrenValueToNum(Value value) {
	return value.as.num;
}

static inline Value wrenNumToValue(double num) {
	Value value;
	value.type = VAL_NUM;
	value.as.num = num;
	return value;
}

#endif
