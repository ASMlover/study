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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void show_array(int* array, int count) {
  int i;

  for (i = 0; i < count; ++i) 
    fprintf(stdout, "%d ", array[i]);
  fprintf(stdout, "\n");
}

static void bubble_sort(int* array, int count) {
  int i, j, t;

  for (i = 0; i < count; ++i) {
    for (j = 0; j < count - i - 1; ++j) {
      if (array[j] > array[j + 1]) {
        t = array[j];
        array[j] = array[j + 1];
        array[j + 1] = t;
      }
    }
  }
}


int main(int argc, char* argv[]) {
#define COUNT 10
#define RSEED 100
  int i;
  int array[COUNT];

  srand((unsigned int)time(NULL));
  for (i = 0; i < COUNT; ++i)
    array[i] = rand() % RSEED;
  show_array(array, COUNT);

  bubble_sort(array, COUNT);
  show_array(array, COUNT);

  return 0;
}
