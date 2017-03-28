/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#include "ms.h"

void ms_sample1(void) {
  fprintf(stdout, "sample1: all objects on stack\n");

  NyVM* vm = NyVM_New();
  NyObject_PushInt(vm, 1);
  NyObject_PushInt(vm, 2);

  NyGC_Collect(vm);
  NyVM_Free(vm);
}

void ms_sample2(void) {
  fprintf(stdout, "sample2: objects unreached\n");

  NyVM* vm = NyVM_New();
  NyObject_PushInt(vm, 1);
  NyObject_PushInt(vm, 2);
  NyObject_Pop(vm);
  NyObject_Pop(vm);

  NyGC_Collect(vm);
  NyVM_Free(vm);
}

void ms_sample3(void) {
  fprintf(stdout, "sample3: reached nested objects\n");

  NyVM* vm = NyVM_New();
  NyObject_PushInt(vm, 1);
  NyObject_PushInt(vm, 2);
  NyObject_PushPair(vm);
  NyObject_PushInt(vm, 3);
  NyObject_PushInt(vm, 4);
  NyObject_PushPair(vm);
  NyObject_PushPair(vm);

  NyGC_Collect(vm);
  NyVM_Free(vm);
}

void ms_sample4(void) {
  fprintf(stdout, "sample4: cycles objects\n");

  NyVM* vm = NyVM_New();
  NyObject_PushInt(vm, 1);
  NyObject_PushInt(vm, 2);
  NyObject* a = NyObject_PushPair(vm);
  NyObject_PushInt(vm, 3);
  NyObject_PushInt(vm, 4);
  NyObject* b = NyObject_PushPair(vm);

  a->tail = b;
  b->tail = a;

  NyGC_Collect(vm);
  NyObject_Pop(vm);
  NyObject_Pop(vm);

  NyGC_Collect(vm);
  NyVM_Free(vm);
}

void ms_sample5(void) {
  fprintf(stdout, "sample5: performace\n");
  NyVM* vm = NyVM_New();

  for (int i = 0; i < 1000000; ++i) {
    for (int j = 0; j < 20; ++j)
      NyObject_PushInt(vm, i);

    for (int j = 0; j < 20; ++j)
      NyObject_Pop(vm);
  }

  NyVM_Free(vm);
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  ms_sample1();
  ms_sample2();
  ms_sample3();
  ms_sample4();
  ms_sample5();

  return 0;
}
