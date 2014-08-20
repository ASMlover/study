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


static void merge_array(
    int* array1, int count1, int* array2, int count2) {
  int i = 0, j = 0, k = 0;
  int* array = (int*)malloc(sizeof(int) * (count1 + count2));

  while (i < count1 && j < count2)
    array[k++] = (array1[i] < array2[j]) ? array1[i++] : array2[j++];

  while (i < count1)
    array[k++] = array1[i++];
  while (j < count2)
    array[k++] = array2[j++];

  for (i = 0; i < (count1 + count2); ++i)
    array1[i] = array[i];

  free(array);
}

static void merge_sort(int* array, int count) {
  if (count > 1) {
    int* array1 = array;
    int count1 = count / 2;
    int* array2 = array + count1;
    int count2 = count - count1;

    merge_sort(array1, count1);
    merge_sort(array2, count2);

    merge_array(array1, count1, array2, count2);
  }
}


static void show_array(int* array, int count) {
  int i;

  for (i = 0; i < count; ++i)
    fprintf(stdout, "%d ", array[i]);
  fprintf(stdout, "\n\n");
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

  merge_sort(array, COUNT);
  show_array(array, COUNT);

  return 0;
}
