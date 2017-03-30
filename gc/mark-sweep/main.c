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
#include "nj_gc.h"

void njord_sample1(void) {
  fprintf(stdout, "sample1: all objects on stack\n");

  NjVM* vm = njord_new();
  njord_pushint(vm, 1);
  njord_pushint(vm, 2);

  njord_collect(vm);
  njord_free(vm);
}

void njord_sample2(void) {
  fprintf(stdout, "sample2: objects unreached\n");

  NjVM* vm = njord_new();
  njord_pushint(vm, 1);
  njord_pushint(vm, 2);
  njord_pop(vm);
  njord_pop(vm);

  njord_collect(vm);
  njord_free(vm);
}

void njord_sample3(void) {
  fprintf(stdout, "sample3: reached nested objects\n");

  NjVM* vm = njord_new();
  njord_pushint(vm, 1);
  njord_pushint(vm, 2);
  njord_pushpair(vm);
  njord_pushint(vm, 3);
  njord_pushint(vm, 4);
  njord_pushpair(vm);
  njord_pushpair(vm);

  njord_collect(vm);
  njord_free(vm);
}

void njord_sample4(void) {
  fprintf(stdout, "sample4: cycles objects\n");

  NjVM* vm = njord_new();
  njord_pushint(vm, 1);
  njord_pushint(vm, 2);
  NjObject* a = njord_pushpair(vm);
  njord_pushint(vm, 3);
  njord_pushint(vm, 4);
  NjObject* b = njord_pushpair(vm);

  a->tail = b;
  b->tail = a;

  njord_collect(vm);
  njord_pop(vm);
  njord_pop(vm);

  njord_collect(vm);
  njord_free(vm);
}

void njord_sample5(void) {
  fprintf(stdout, "sample5: performace\n");
  NjVM* vm = njord_new();

  for (int i = 0; i < 1000000; ++i) {
    for (int j = 0; j < 20; ++j)
      njord_pushint(vm, i);

    for (int j = 0; j < 20; ++j)
      njord_pop(vm);
  }

  njord_free(vm);
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  njord_sample1();
  njord_sample2();
  njord_sample3();
  njord_sample4();
  njord_sample5();

  return 0;
}
