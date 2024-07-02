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
#include <string.h>
#include <time.h>
#include "opt_random.h"
#include "wren.h"
#include "vm.h"
#include "opt_random.wren.inc"

// Implements the well equidistributed long-period linear PRNG (WELL512a).
//
// https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear
typedef struct {
	u32_t state[16];
	u32_t index;
} Well512;

// Code from: http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
static inline u32_t advanceState(Well512* well) {
	u32_t a, b, c, d;
	a = well->state[well->index];
	c = well->state[(well->index + 13) & 15];
	b = a ^ c ^ (a << 16) ^ (c << 15);
	c = well->state[(well->index + 9) & 15];
	c ^= (c >> 11);
	a = well->state[well->index] = b ^ c;
	d = a ^ ((a << 5) & 0xda442d24U);

	well->index = (well->index + 15) & 15;
	a = well->state[well->index];
	well->state[well->index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
	return well->state[well->index];
}

static void randomSeed0(WrenVM* vm) {
	Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

	srand((u32_t)time(NULL));
	for (int i = 0; i < 16; ++i)
		well->state[i] = rand();
}

static void randomSeed1(WrenVM* vm) {
	Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

	srand((u32_t)wrenGetSlotDouble(vm, 1));
	for (int i = 0; i < 16; ++i)
		well->state[i] = rand();
}

static void randomSeed16(WrenVM* vm) {
	Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);
	for (int i = 0; i < 16; ++i)
		well->state[i] = (u32_t)wrenGetSlotDouble(vm, i + 1);
}

static void randomFloat(WrenVM* vm) {
	Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);

	double result = (double)advanceState(well) * (1 << 21);
	result += (double)(advanceState(well) & ((1 << 21) - 1));
	result /= 9007199254740992.0;

	wrenSetSlotDouble(vm, 0, result);
}

static void randomInt0(WrenVM* vm) {
	Well512* well = (Well512*)wrenGetSlotForeign(vm, 0);
	wrenSetSlotDouble(vm, 0, (double)advanceState(well));
}

static void randomAllocate(WrenVM* vm) {
	Well512* well = (Well512*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Well512));
	well->index = 0;
}

const char* wrenRandomSource() {
	return randomModuleSource;
}

WrenForeignClassMethods wrenRandomBindForeignClass(WrenVM* vm, const char* module, const char* className) {
	ASSERT(0 == strcmp(className, "Random"), "Should be in Random class.");
	WrenForeignClassMethods methods;
	methods.allocate = randomAllocate;
	methods.finalize = NULL;
	return methods;
}

WrenForeignMethodFn wrenRandomBindForeignMethod(WrenVM* vm,
		const char* className, bool isStatic, const char* signature) {
	ASSERT(0 == strcmp(className, "Random"), "Should be in Random class.");
	if (0 == strcmp(signature, "<allocate>"))
		return randomAllocate;
	if (0 == strcmp(signature, "seed_()"))
		return randomSeed0;
	if (0 == strcmp(signature, "seed_(_)"))
		return randomSeed1;
	if (0 == strcmp(signature, "seed_(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)"))
		return randomSeed16;
	if (0 == strcmp(signature, "float()"))
		return randomFloat;
	if (0 == strcmp(signature, "int()"))
		return randomInt0;

	ASSERT(false, "Unknown method.");
	return NULL;
}
