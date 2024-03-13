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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "vm.h"

#define WREN_MIN_CAPACITY               (16)
#define WREN_GROW_FACTOR                (2)
#define WREN_MAP_LOAD_PERCENT           (75)
#define WREN_INITIAL_CALL_FRAMES        (4)

DEFINE_BUFFER(Value, Value);
DEFINE_BUFFER(Method, Method);

static void initObj(WrenVM* vm, Obj* obj, ObjType type, ObjClass* classObj) {
	obj->type = type;
	obj->isDark = false;
	obj->classObj = classObj;
	obj->next = vm->first;
	vm->first = obj;
}

static inline u32_t hashBits(DoubleBits bits) {
	u32_t result = bits.bits32[0] ^ bits.bits32[1];
	result ^= (result >> 20) ^ (result >> 12);
	result ^= (result >> 7) ^ (result >> 4);
	return result;
}

static inline u32_t hashNumber(double num) {
	DoubleBits bits;
	bits.num = num;
	return hashBits(bits);
}

static u32_t hashObject(Obj* object) {
	switch (object->type) {
	case OBJ_CLASS: return hashObject((Obj*)((ObjClass*)object)->name);
	case OBJ_FN:
		{
			ObjFn* fn = (ObjFn*)object;
			return hashNumber(fn->arity) ^ hashNumber(fn->code.count);
		} break;
	case OBJ_RANGE:
		{
			ObjRange* range = (ObjRange*)object;
			return hashNumber(range->from) ^ hashNumber(range->to);
		} break;
	case OBJ_STRING: return ((ObjString*)object)->hash;
	default: ASSERT(false, "Only immutable objects can be hashed."); break;
	}
	return 0;
}

static u32_t hashValue(Value value) {
	switch (value.type) {
	case VAL_FALSE: return 0;
	case VAL_NULL:  return 1;
	case VAL_NUM:   return hashNumber(AS_NUM(value));
	case VAL_TRUE:  return 2;
	case VAL_OBJ:   return hashObject(AS_OBJ(value));
	default:        UNREACHABLE();
	}
	return 0;
}

static bool findEntry(MapEntry* entries, u32_t capacity, Value key, MapEntry** result) {
	if (capacity == 0)
		return false;

	u32_t startIndex = hashValue(key) % capacity;
	u32_t index = startIndex;

	MapEntry* tombstone = NULL;
	do {
		MapEntry* entry = &entries[index];
		if (IS_UNDEFINED(entry->key)) {
			if (IS_FALSE(entry->value)) {
				*result = tombstone != NULL ? tombstone : entry;
				return false;
			}
			else {
				if (tombstone == NULL)
					tombstone = entry;
			}
		}
		else if (wrenValuesEqual(entry->key, key)) {
			*result = entry;
			return true;
		}

		index = (index + 1) % capacity;
	} while (index != startIndex);

	ASSERT(tombstone != NULL, "Map should have tombstones or empty entries.");
	*result = tombstone;
	return false;
}

static bool insertEntry(MapEntry* entries, u32_t capacity, Value key, Value value) {
	ASSERT(entries != NULL, "Should ensure capacity before inserting.");

	MapEntry* entry;
	if (findEntry(entries, capacity, key, &entry)) {
		entry->value = value;
		return false;
	}
	else {
		entry->key = key;
		entry->value = value;
		return true;
	}
}

static void resizeMap(WrenVM* vm, ObjMap* map, u32_t capacity) {
	MapEntry* entries = ALLOCATE_ARRAY(vm, MapEntry, capacity);
	for (u32_t i = 0; i < capacity; ++i) {
		entries[i].key = UNDEFINED_VAL;
		entries[i].value = FALSE_VAL;
	}

	if (map->capacity > 0) {
		for (u32_t i = 0; i < map->capacity; ++i) {
			MapEntry* entry = &map->entries[i];
			if (IS_UNDEFINED(entry->key))
				continue;

			insertEntry(entries, capacity, entry->key, entry->value);
		}
	}

	DEALLOCATE(vm, map->entries);
	map->entries = entries;
	map->capacity = capacity;
}

static ObjString* allocateString(WrenVM* vm, sz_t length) {
	ObjString* string = ALLOCATE_FLEX(vm, ObjString, char, length + 1);
	initObj(vm, &string->obj, OBJ_STRING, vm->stringClass);

	string->length = (int)length;
	string->value[length] = 0;

	return string;
}

static void hashString(ObjString* string) {
	// FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/
	u32_t hash = 2166136261u;
	for (u32_t i = 0; i < string->length; ++i) {
		hash ^= string->value[i];
		hash *= 16777619;
	}
	string->hash = hash;
}

ObjClass* wrenNewSingleClass(WrenVM* vm, int numFields, ObjString* name) {
	ObjClass* classObj = ALLOCATE(vm, ObjClass);
	initObj(vm, &classObj->obj, OBJ_CLASS, NULL);
	classObj->superclass = NULL;
	classObj->numFields = numFields;
	classObj->name = name;

	wrenPushRoot(vm, (Obj*)classObj);
	wrenMethodBufferInit(&classObj->methods);
	wrenPopRoot(vm);

	return classObj;
}

void wrenBindSuperclass(WrenVM* vm, ObjClass* subclass, ObjClass* superclass) {
	ASSERT(superclass != NULL, "Must have superclass.");

	subclass->superclass = superclass;
	if (subclass->numFields != -1)
		subclass->numFields += superclass->numFields;
	else
		ASSERT(superclass->numFields == 0, "A foreign class cannot inherit from a class with fields.");

	for (int i = 0; i < superclass->methods.count; ++i)
		wrenBindMethod(vm, subclass, i, superclass->methods.data[i]);
}

ObjClass* wrenNewClass(WrenVM* vm, ObjClass* superclass, int numFields, ObjString* name) {
	Value metaclassName = wrenStringFormat(vm, "@ metaclass", OBJ_VAL(name));

	wrenPushRoot(vm, AS_OBJ(metaclassName));
	ObjClass* metaclass = wrenNewSingleClass(vm, 0, AS_STRING(metaclassName));
	wrenPopRoot(vm);

	wrenPushRoot(vm, (Obj*)metaclass);
	wrenBindSuperclass(vm, metaclass, vm->classClass);
	ObjClass* classObj = wrenNewSingleClass(vm, numFields, name);

	wrenPushRoot(vm, (Obj*)classObj);
	classObj->obj.classObj = metaclass;
	wrenBindSuperclass(vm, classObj, superclass);

	wrenPopRoot(vm);
	wrenPopRoot(vm);

	return classObj;
}

void wrenBindMethod(WrenVM* vm, ObjClass* classObj, int symbol, Method method) {
	if (symbol >= classObj->methods.count) {
		Method noMethod;
		noMethod.type = METHOD_NONE;
		wrenMethodBufferFill(vm, &classObj->methods, noMethod, symbol - classObj->methods.count + 1);
	}
	classObj->methods.data[symbol] = method;
}

ObjClosure* wrenNewClosure(WrenVM* vm, ObjFn* fn) {
	ObjClosure* closure = ALLOCATE_FLEX(vm, ObjClosure, ObjUpvalue*, fn->numUpvalues);
	initObj(vm, &closure->obj, OBJ_CLOSURE, vm->fnClass);
	closure->fn = fn;

	for (int i = 0; i < fn->numUpvalues; ++i)
		closure->upvalues[i] = NULL;
	return closure;
}

ObjFiber* wrenNewFiber(WrenVM* vm, ObjClosure* closure) {
	CallFrame* frames = ALLOCATE_ARRAY(vm, CallFrame, WREN_INITIAL_CALL_FRAMES);

	int stackCapacity = closure == NULL ? 1 : wrenPowerOf2Ceil(closure->fn->maxSlots + 1);
	Value* stack = ALLOCATE_ARRAY(vm, Value, stackCapacity);

	ObjFiber* fiber = ALLOCATE(vm, ObjFiber);
	initObj(vm, &fiber->obj, OBJ_FIBER, vm->fiberClass);

	fiber->stack = stack;
	fiber->stackTop = fiber->stack;
	fiber->stackCapacity = stackCapacity;

	fiber->frames = frames;
	fiber->frameCapacity = WREN_INITIAL_CALL_FRAMES;
	fiber->numFrames = 0;

	fiber->openUpvalues = NULL;
	fiber->caller = NULL;
	fiber->error = NULL_VAL;
	fiber->state = FIBER_OTHER;

	if (closure != NULL) {
		wrenAppendCallFrame(vm, fiber, closure, fiber->stack);
		*fiber->stackTop++ = OBJ_VAL(closure);
	}

	return fiber;
}

void wrenEnsureStack(WrenVM* vm, ObjFiber* fiber, int needed) {
	if (fiber->stackCapacity >= needed)
		return;

	int newCapacity = wrenPowerOf2Ceil(needed);
	Value* oldStack = fiber->stack;
	fiber->stack = (Value*)wrenReallocate(vm, fiber->stack,
			sizeof(Value) * fiber->stackCapacity, sizeof(Value) * newCapacity);
	fiber->stackCapacity = newCapacity;

	if (fiber->stack != oldStack) {
		if (vm->apiStack >= oldStack && vm->apiStack <= fiber->stackTop)
			vm->apiStack = fiber->stack + (vm->apiStack - oldStack);

		for (int i = 0; i < fiber->numFrames; ++i) {
			CallFrame* frame = &fiber->frames[i];
			frame->stackStart = fiber->stack + (frame->stackStart - oldStack);
		}

		for (ObjUpvalue* upvalue = fiber->openUpvalues; upvalue != NULL; upvalue = upvalue->next)
			upvalue->value = fiber->stack + (upvalue->value - oldStack);
		fiber->stackTop = fiber->stack + (fiber->stackTop - oldStack);
	}
}

ObjForeign* wrenNewForeign(WrenVM* vm, ObjClass* classObj, sz_t size) {
	ObjForeign* object = ALLOCATE_FLEX(vm, ObjForeign, u8_t, size);
	initObj(vm, &object->obj, OBJ_FOREIGN, classObj);

	memset(object->data, 0, size);
	return object;
}

ObjFn* wrenNewFunction(WrenVM* vm, ObjModule* module, int maxSlots) {
	FnDebug* debug = ALLOCATE(vm, FnDebug);
	debug->name = NULL;
	wrenIntBufferInit(&debug->sourceLines);

	ObjFn* fn = ALLOCATE(vm, ObjFn);
	initObj(vm, &fn->obj, OBJ_FN, vm->fnClass);

	wrenValueBufferInit(&fn->constants);
	wrenByteBufferInit(&fn->code);
	fn->module = module;
	fn->maxSlots = maxSlots;
	fn->numUpvalues = 0;
	fn->arity = 0;
	fn->debug = debug;

	return fn;
}

void wrenFunctionBindName(WrenVM* vm, ObjFn* fn, const char* name, int length) {
	fn->debug->name = ALLOCATE_ARRAY(vm, char, length + 1);
	memcpy(fn->debug->name, name, length);
	fn->debug->name[length] = 0;
}

Value wrenNewInstance(WrenVM* vm, ObjClass* classObj) {
	ObjInstance* instance = ALLOCATE_FLEX(vm, ObjInstance, Value, classObj->numFields);
	initObj(vm, &instance->obj, OBJ_INSTANCE, classObj);

	for (int i = 0; i < classObj->numFields; ++i)
		instance->fields[i] = NULL_VAL;
	return OBJ_VAL(instance);
}

ObjList* wrenNewList(WrenVM* vm, u32_t numElements) {
	Value* elements = NULL;
	if (numElements > 0)
		elements = ALLOCATE_ARRAY(vm, Value, numElements);

	ObjList* list = ALLOCATE(vm, ObjList);
	initObj(vm, &list->obj, OBJ_LIST, vm->listClass);
	list->elements.count = numElements;
	list->elements.capacity = numElements;
	list->elements.data = elements;
	return list;
}

void wrenListInsert(WrenVM* vm, ObjList* list, Value value, u32_t index) {
	if (IS_OBJ(value))
		wrenPushRoot(vm, AS_OBJ(value));

	wrenValueBufferWrite(vm, &list->elements, NULL_VAL);

	if (IS_OBJ(value))
		wrenPopRoot(vm);

	for (u32_t i = list->elements.count - 1; i > index; --i)
		list->elements.data[i] = list->elements.data[i - 1];
	list->elements.data[index] = value;
}

Value wrenListRemoveAt(WrenVM* vm, ObjList* list, u32_t index) {
	Value removed = list->elements.data[index];

	if (IS_OBJ(removed))
		wrenPushRoot(vm, AS_OBJ(removed));

	for (int i = index; i < list->elements.count - 1; ++i)
		list->elements.data[i] = list->elements.data[i + 1];

	int newCapacity = list->elements.capacity / WREN_GROW_FACTOR;
	if (newCapacity >= list->elements.count) {
		list->elements.data = (Value*)wrenReallocate(vm, list->elements.data,
				sizeof(Value) * list->elements.capacity, sizeof(Value) * newCapacity);
		list->elements.capacity = newCapacity;
	}

	if (IS_OBJ(removed))
		wrenPopRoot(vm);

	--list->elements.count;
	return removed;
}

ObjMap* wrenNewMap(WrenVM* vm) {
	ObjMap* map = ALLOCATE(vm, ObjMap);
	initObj(vm, &map->obj, OBJ_MAP, vm->mapClass);
	map->count = 0;
	map->capacity = 0;
	map->entries = NULL;
	return map;
}

void wrenMapClear(WrenVM* vm, ObjMap* map) {
	DEALLOCATE(vm, map->entries);
	map->entries = NULL;
	map->count = 0;
	map->capacity = 0;
}

Value wrenMapGet(ObjMap* map, Value key) {
	MapEntry* entry;
	if (findEntry(map->entries, map->capacity, key, &entry))
		return entry->value;
	return UNDEFINED_VAL;
}

void wrenMapSet(WrenVM* vm, ObjMap* map, Value key, Value value) {
	if (map->count + 1 > map->capacity * WREN_MAP_LOAD_PERCENT / 100) {
		u32_t capacity = map->capacity * WREN_GROW_FACTOR;
		if (capacity < WREN_MIN_CAPACITY)
			capacity = WREN_MIN_CAPACITY;
		resizeMap(vm, map, capacity);
	}

	if (insertEntry(map->entries, map->capacity, key, value))
		++map->count;
}

Value wrenMapRemoveKey(WrenVM* vm, ObjMap* map, Value key) {
	MapEntry* entry;
	if (!findEntry(map->entries, map->capacity, key, &entry))
		return NULL_VAL;

	Value value = entry->value;
	entry->key = UNDEFINED_VAL;
	entry->value = TRUE_VAL;

	if (IS_OBJ(value))
		wrenPushRoot(vm, AS_OBJ(value));

	--map->count;
	if (map->count == 0) {
		wrenMapClear(vm, map);
	}
	else if (map->capacity > WREN_MIN_CAPACITY &&
			map->count < map->capacity / WREN_GROW_FACTOR * WREN_MAP_LOAD_PERCENT / 100) {
		u32_t capacity = map->capacity / WREN_GROW_FACTOR;
		if (capacity < WREN_MIN_CAPACITY)
			capacity = WREN_MIN_CAPACITY;

		resizeMap(vm, map, capacity);
	}

	if (IS_OBJ(value))
		wrenPopRoot(vm);

	return value;
}

ObjModule* wrenNewModule(WrenVM* vm, ObjString* name) {
	ObjModule* module = ALLOCATE(vm, ObjModule);
	initObj(vm, (Obj*)module, OBJ_MODULE, NULL);

	wrenPushRoot(vm, (Obj*)module);

	wrenSymbolTableInit(&module->variableNames);
	wrenValueBufferInit(&module->variables);
	module->name = name;

	wrenPopRoot(vm);
	return module;
}

Value wrenNewRange(WrenVM* vm, double from, double to, bool isInclusive) {
	ObjRange* range = ALLOCATE(vm, ObjRange);
	initObj(vm, &range->obj, OBJ_RANGE, vm->rangeClass);

	range->from = from;
	range->to = to;
	range->isInclusive = isInclusive;

	return OBJ_VAL(range);
}

Value wrenNewString(WrenVM* vm, const char* text) {
	return wrenNewStringLength(vm, text, strlen(text));
}

Value wrenNewStringLength(WrenVM* vm, const char* text, sz_t length) {
	ASSERT(length == 0 || text != NULL, "Unexpected NULL string.");

	ObjString* string = allocateString(vm, length);
	if (length > 0 && text != NULL)
		memcpy(string->value, text, length);
	hashString(string);

	return OBJ_VAL(string);
}

Value wrenNewStringFromRange(WrenVM* vm, ObjString* source, int start, u32_t count, int step) {
	u8_t* from = (u8_t*)source->value;
	int length = 0;
	for (u32_t i = 0; i < count; ++i)
		length += wrenUtf8DecodeNumBytes(from[start + i * step]);

	ObjString* result = allocateString(vm, length);
	result->value[length] = 0;
	u8_t* to = (u8_t*)result->value;
	for (u32_t i = 0; i < count; ++i) {
		int index = start + i * step;
		int codePoint = wrenUtf8Decode(from + index, source->length - index);

		if (codePoint != -1)
			to += wrenUtf8Encode(codePoint, to);
	}
	hashString(result);

	return OBJ_VAL(result);
}

Value wrenNumToString(WrenVM* vm, double value) {
	if (isnan(value))
		return CONST_STRING(vm, "nan");
	if (isinf(value)) {
		if (value > 0.0)
			return CONST_STRING(vm, "infinity");
		else
			return CONST_STRING(vm, "-infinity");
	}

	char buffer[24];
	int length = sprintf(buffer, "%.14g", value);
	return wrenNewStringLength(vm, buffer, length);
}

Value wrenStringFormat(WrenVM* vm, const char* format, ...) {
	va_list argList;

	va_start(argList, format);
	sz_t totalLength = 0;
	for (const char* c = format; *c != 0; ++c) {
		switch (*c) {
		case '$': totalLength += strlen(va_arg(argList, const char*)); break;
		case '@': totalLength += AS_STRING(va_arg(argList, Value))->length; break;
		default: ++totalLength; break;
		}
	}
	va_end(argList);

	ObjString* result = allocateString(vm, totalLength);
	va_start(argList, format);
	char* start = result->value;
	for (const char* c = format; *c != 0; ++c) {
		switch (*c) {
		case '$':
			{
				const char* string = va_arg(argList, const char*);
				sz_t length = strlen(string);
				memcpy(start, string, length);
				start += length;
			} break;
		case '@':
			{
				ObjString* string = AS_STRING(va_arg(argList, Value));
				memcpy(start, string->value, string->length);
				start += string->length;
			} break;
		default: *start++ = *c; break;
		}
	}
	va_end(argList);

	hashString(result);

	return OBJ_VAL(result);
}

Value wrenStringFromCodePoint(WrenVM* vm, int value) {
	int length = wrenUtf8EncodeNumBytes(value);
	ASSERT(length != 0, "Value out of range.");

	ObjString* string = allocateString(vm, length);
	wrenUtf8Encode(value, (u8_t*)string->value);
	hashString(string);

	return OBJ_VAL(string);
}

Value wrenStringFromByte(WrenVM* vm, u8_t value) {
	int length = 1;
	ObjString* string = allocateString(vm, length);
	string->value[0] = value;
	hashString(string);

	return OBJ_VAL(string);
}

Value wrenStringCodePointAt(WrenVM* vm, ObjString* string, u32_t index) {
	ASSERT(index < string->length, "Index out of bounds.");

	int codePoint = wrenUtf8Decode((u8_t*)string->value + index, string->length - index);
	if (codePoint == -1) {
		char bytes[2];
		bytes[0] = string->value[index];
		bytes[1] = 0;
		return wrenNewStringLength(vm, bytes, 1);
	}
	return wrenStringFromCodePoint(vm, codePoint);
}

u32_t wrenStringFind(ObjString* haystack, ObjString* needle, u32_t start) {
	if (needle->length == 0)
		return start;
	if (start + needle->length > haystack->length)
		return UINT32_MAX;
	if (start >= haystack->length)
		return UINT32_MAX;

	u32_t shift[UINT8_MAX];
	u32_t needleEnd = needle->length - 1;
	for (u32_t index = 0; index < UINT8_MAX; ++index)
		shift[index] = needle->length;

	for (u32_t index = 0; index < needleEnd; ++index) {
		char c = needle->value[index];
		shift[(u8_t)c] = needleEnd - index;
	}

	char lastChar = needle->value[needleEnd];
	u32_t range = haystack->length - needle->length;

	for (u32_t index = start; index <= range;) {
		char c = haystack->value[index + needleEnd];
		if (lastChar == c && memcmp(haystack->value + index, needle->value, needleEnd) == 0)
			return index;

		index += shift[(u8_t)c];
	}
	return UINT32_MAX;
}

ObjUpvalue* wrenNewUpvalue(WrenVM* vm, Value* value) {
	ObjUpvalue* upvalue = ALLOCATE(vm, ObjUpvalue);
	initObj(vm, &upvalue->obj, OBJ_UPVALUE, NULL);

	upvalue->value = value;
	upvalue->closed = NULL_VAL;
	upvalue->next = NULL;
	return upvalue;
}

void wrenGrayObj(WrenVM* vm, Obj* obj) {}
void wrenGrayValue(WrenVM* vm, Value value) {}
void wrenGrayBuffer(WrenVM* vm, ValueBuffer* buffer) {}
void wrenBlackenObjects(WrenVM* vm) {}
void wrenFreeObj(WrenVM* vm, Obj* obj) {}
ObjClass* wrenGetClass(WrenVM* vm, Value value) { return NULL; }
bool wrenValuesEqual(Value a, Value b) { return false; }
