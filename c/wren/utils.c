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
#include "utils.h"
#include "vm.h"

DEFINE_BUFFER(Byte, u8_t);
DEFINE_BUFFER(Int, int);
DEFINE_BUFFER(String, ObjString*);

void wrenSymbolTableInit(SymbolTable* symbols) {}
void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols) {}
void wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols, const char* name, sz_t length) {}
int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols, const char* name, sz_t length) { return 0; }
void wrenBlackenSymbolTable(WrenVM* vm, SymbolTable* symbols) {}

int wrenUtf8EncodeNumBytes(int value) {
	ASSERT(value >= 0, "Cannot encode a negative value.");

	if (value <= 0x7f)
		return 1;
	if (value <= 0x7ff)
		return 2;
	if (value <= 0xffff)
		return 3;
	if (value <= 0x10ffff)
		return 4;
	return 0;
}

int wrenUtf8Encode(int value, u8_t* bytes) {
	// [REF] https://datatracker.ietf.org/doc/html/rfc3629
	if (value <= 0x7f) {
		// Single byte (fits in ASCII)
		*bytes = value & 0x7f;
		return 1;
	}
	else if (value <= 0x7ff) {
		// Two byte sequence: 110xxxxx 10xxxxxx
		*bytes = 0xc0 | ((value & 0x7c0) >> 6);
		++bytes;
		*bytes = 0x80 | (value & 0x3f);
		return 2;
	}
	else if (value <= 0xffff) {
		// Three byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
		*bytes = 0xe0 | ((value & 0xf000) >> 12);
		++bytes;
		*bytes = 0x80 | ((value & 0xfc0) >> 6);
		++bytes;
		*bytes = 0x80 | (value & 0x3f);
		return 3;
	}
	else if (value <= 0x10ffff) {
		*bytes = 0xf0 | ((value & 0x1c0000) >> 18);
		++bytes;
		*bytes = 0x80 | ((value & 0x3f000) >> 12);
		++bytes;
		*bytes = 0x80 | ((value & 0xfc0) >> 6);
		++bytes;
		*bytes = 0x80 | (value & 0x3f);
		return 4;
	}

	// Invalid Unicode value, see: https://datatracker.ietf.org/doc/html/rfc3629
	UNREACHABLE();
	return 0;
}

int wrenUtf8DecodeNumBytes(u8_t byte) {
	if ((byte & 0xc0) == 0x80)
		return 0;
	if ((byte & 0xf8) == 0xf0)
		return 4;
	if ((byte & 0xf0) == 0xe0)
		return 3;
	if ((byte & 0xe0) == 0xc0)
		return 2;
	return 1;
}

int wrenUtf8Decode(const u8_t* bytes, u32_t length) {
	// Single byte (fits in ASCII)
	if (*bytes <= 0x7f)
		return *bytes;

	int value;
	u32_t remainingBytes;
	if ((*bytes & 0xe0) == 0xc0) {
		// Two byte sequence: 110xxxxx 10xxxxxx
		value = *bytes & 0x1f;
		remainingBytes = 1;
	}
	else if ((*bytes & 0xf0) == 0xe0) {
		// Three byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
		value = *bytes & 0x0f;
		remainingBytes = 2;
	}
	else if ((*bytes & 0xf8) == 0xf0) {
		// Four byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		value = *bytes & 0x07;
		remainingBytes = 3;
	}
	else {
		// Invalid UTF-8 sequence
		return -1;
	}

	if (remainingBytes > length - 1)
		return -1;
	while (remainingBytes > 0) {
		++bytes;
		--remainingBytes;

		// Remaining bytes must be of form 10xxxxxx
		if ((*bytes & 0xc0) != 0x80)
			return -1;
		value = value << 6 | (*bytes & 0x3f);
	}
	return value;
}

int wrenPowerOf2Ceil(int n) {
	// [REF] http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	++n;

	return n;
}
