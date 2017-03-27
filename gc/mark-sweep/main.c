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

  VM* vm = new_vm();
  push_int(vm, 1);
  push_int(vm, 2);

  gc_collect(vm);
  free_vm(vm);
}

void ms_sample2(void) {
  fprintf(stdout, "sample2: objects unreached\n");

  VM* vm = new_vm();
  push_int(vm, 1);
  push_int(vm, 2);
  pop_object(vm);
  pop_object(vm);

  gc_collect(vm);
  free_vm(vm);
}

void ms_sample3(void) {
  fprintf(stdout, "sample3: reached nested objects\n");

  VM* vm = new_vm();
  push_int(vm, 1);
  push_int(vm, 2);
  push_pair(vm);
  push_int(vm, 3);
  push_int(vm, 4);
  push_pair(vm);
  push_pair(vm);

  gc_collect(vm);
  free_vm(vm);
}

void ms_sample4(void) {
  fprintf(stdout, "sample4: cycles objects\n");

  VM* vm = new_vm();
  push_int(vm, 1);
  push_int(vm, 2);
  Object* a = push_pair(vm);
  push_int(vm, 3);
  push_int(vm, 4);
  Object* b = push_pair(vm);

  a->first = b;
  b->first = a;

  gc_collect(vm);
  pop_object(vm);
  pop_object(vm);

  gc_collect(vm);
  free_vm(vm);
}

void ms_sample5(void) {
  fprintf(stdout, "sample5: performace\n");
  VM* vm = new_vm();

  for (int i = 0; i < 1000000; ++i) {
    for (int j = 0; j < 20; ++j)
      push_int(vm, i);

    for (int j = 0; j < 20; ++j)
      pop_object(vm);
  }

  free_vm(vm);
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
