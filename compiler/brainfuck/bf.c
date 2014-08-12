/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TOKENS "><+-.,[]"

#define CODE_SEGMENT_SIZE 30000
#define STACK_SEGMENT_SIZE 1000
#define DATA_SEGMENT_SIZE 30000

typedef void (*Callback)(void);

struct {
  char cs[CODE_SEGMENT_SIZE];   /* Code Segment */
  long ip;                      /* Instruction Pointer */

  char ss[STACK_SEGMENT_SIZE];  /* Stack Segment */
  long sp;                      /* Stack Pointer */

  char ds[DATA_SEGMENT_SIZE];   /* Data Segment */
  long bp;                      /* Base Pointer */

  Callback fn[128];
} vm;

static void vm_forward(void) {  
  vm.bp = (vm.bp + 1) % DATA_SEGMENT_SIZE;
}

static void vm_backward(void) {  
  vm.bp = (vm.bp + DATA_SEGMENT_SIZE - 1) % DATA_SEGMENT_SIZE;
}

static void vm_increment(void) {
  vm.ds[vm.bp]++;
}

static void vm_decrement(void) {
  vm.ds[vm.bp]--;
}

static void vm_input(void) {
  vm.ds[vm.bp] = getchar();
}

static void vm_output(void) {
  putchar(vm.ds[vm.bp]);
}

static void vm_while_entry(void) {
  if (vm.ds[vm.bp]) {
    vm.ss[vm.sp] = (char)(vm.ip - 1);
    vm.sp++;
  } 
  else {
    int c = 1;
    for (vm.ip++; vm.cs[vm.ip] && c; vm.ip++) {
      if (vm.cs[vm.ip] == '[') {
        c++;
      } 
      else if (vm.cs[vm.ip] == ']') {
        c--;
      }
    }
  }
}

static void vm_while_exit(void) {
  if (vm.ds[vm.bp]) {
    vm.sp--;
    vm.ip = vm.ss[vm.sp];
  }
}

static void init(void) {
  memset(&vm, 0, sizeof(vm));
  vm.fn['>'] = vm_forward;
  vm.fn['<'] = vm_backward;
  vm.fn['+'] = vm_increment;
  vm.fn['-'] = vm_decrement;
  vm.fn['.'] = vm_output;
  vm.fn[','] = vm_input;
  vm.fn['['] = vm_while_entry;
  vm.fn[']'] = vm_while_exit;
}

void setup(FILE* stream) {
  int c;
  int i;

  if (NULL == stream)
    return;
  init();
  for (i = 0; (c = fgetc(stream)) != EOF;) {
    if (strchr(TOKENS, c)) {
      vm.cs[i] = c;
      i++;
    }
  }
}

void run(void) {
  while (vm.cs[vm.ip]) {
    vm.fn[vm.cs[vm.ip]]();
    vm.ip++;
  }
}

int main(int argc, char* argv[]) {
  FILE* stream = stdin;
  int file_flag = 0;

  if (argc > 1) {
    stream = fopen(argv[1], "r");
    if (NULL != stream)
      file_flag = 1;
    else 
      stream = stdin;
  }

  setup(stream);
  run();

  if (file_flag)
    fclose(stream);

  return 0;
}

