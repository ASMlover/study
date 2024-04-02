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
#ifndef WREN_UTILS_H
#define WREN_UTILS_H

#include "wren.h"
#include "common.h"

typedef struct ObjString ObjString;

#define DECLARE_BUFFER(name, type)\
	typedef struct {\
		type* data;\
		int count;\
		int capacity;\
	} name##Buffer;\
	void wren##name##BufferInit(name##Buffer* buffer);\
	void wren##name##BufferClear(WrenVM* vm, name##Buffer* buffer);\
	void wren##name##BufferFill(WrenVM* vm, name##Buffer* buffer, type data, int count);\
	void wren##name##BufferWrite(WrenVM* vm, name##Buffer* buffer, type data)

#define DEFINE_BUFFER(name, type)\
	void wren##name##BufferInit(name##Buffer* buffer) {\
		buffer->data = NULL;\
		buffer->count = 0;\
		buffer->capacity = 0;\
	}\
	void wren##name##BufferClear(WrenVM* vm, name##Buffer* buffer) {\
		wrenReallocate(vm, buffer->data, 0, 0);\
		wren##name##BufferInit(buffer);\
	}\
	void wren##name##BufferFill(WrenVM* vm, name##Buffer* buffer, type data, int count) {\
		if (buffer->capacity < buffer->count + count) {\
			int capacity = wrenPowerOf2Ceil(buffer->count + count);\
			buffer->data = (type*)wrenReallocate(vm, buffer->data,\
					buffer->capacity * sizeof(type), capacity * sizeof(type));\
			buffer->capacity = capacity;\
		}\
		for (int i = 0; i < count; ++i)\
			buffer->data[buffer->count++] = data;\
	}\
	void wren##name##BufferWrite(WrenVM* vm, name##Buffer* buffer, type data) {\
		wren##name##BufferFill(vm, buffer, data, 1);\
	}

DECLARE_BUFFER(Byte, u8_t);
DECLARE_BUFFER(Int, int);
DECLARE_BUFFER(String, ObjString*);

typedef StringBuffer SymbolTable;

void wrenSymbolTableInit(SymbolTable* symbols);
void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols);
int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols, const char* name, sz_t length);
int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols, const char* name, sz_t length);
int wrenSymbolTableFind(const SymbolTable* symbols, const char* name, sz_t length);
void wrenBlackenSymbolTable(WrenVM* vm, SymbolTable* symbols);

int wrenUtf8EncodeNumBytes(int value);
int wrenUtf8Encode(int value, u8_t* bytes);
int wrenUtf8DecodeNumBytes(u8_t byte);
int wrenUtf8Decode(const u8_t* bytes, u32_t length);

int wrenPowerOf2Ceil(int n);

#endif
