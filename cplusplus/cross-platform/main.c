/*
 * Copyright (c) 2016 ASMlover. All rights reserved.
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
#include "kp_thread.h"

static void* _kp_thread_routine(void* arg)
{
  int value = (int)arg;
  fprintf(stdout, "kp_thread_t: create success, @value: %d\n", value);

  return NULL;
}

static void _kp_test_thread(void)
{
  kp_thread_t thread = 0;

  fprintf(stdout, "\n#################### kp_thread ####################\n");
  kp_thread_create(&thread, _kp_thread_routine, (void*)4556);
  kp_thread_join(thread);
  thread = 0;
}

static void _kp_thread_key_destructor(void* p) {
  free(p);
}

static void _kp_test_thread_key(void)
{
  kp_thread_key_t key;
  int* value;

  fprintf(stdout, "\n#################### kp_thead_key ####################\n");
  kp_thread_key_create(&key, _kp_thread_key_destructor);
  value = (int*)malloc(sizeof(int));
  *value = 1234;
  kp_thread_setspecific(key, (const void*)value);
  fprintf(stdout, "kp_thread_key_t: getspecific: %d\n", *(int*)kp_thread_getspecific(key));
  kp_thread_key_delete(key);
}

int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  _kp_test_thread();
  _kp_test_thread_key();

  return 0;
}
