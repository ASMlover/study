/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#include <string.h>
#include "sl_test_header.h"

typedef struct sl_test_t {
  const char* cmd;
  void (*test)(void);
} sl_test_t;


static inline void 
sl_help(void)
{
  fprintf(stdout, 
      "usage: slib-test.exe [options] ...\n\n"
      "   help    show help for a given topic or a help overview\n"
      "   alloc   show test result of allocator module\n"
      "   queue   show test result of queue module\n"
      "   list    show test result of list module\n"
      "   table   show test result of table module\n"
      "   array   show test result of array module\n"
      );
}


static const sl_test_t _s_tests[] = {
  {"help", sl_help}, 
  {"alloc", sl_test_allocator}, 
  {"queue", sl_test_queue}, 
  {"list", sl_test_list}, 
  {"table", sl_test_table},
  {"array", sl_test_array}, 
};



int 
main(int argc, char* argv[])
{
  if (argc < 2)
    sl_help();
  else {
    int i, found = 0;
    int len = countof(_s_tests);
    for (i = 0; i < len; ++i) {
      if (cmdeq(argv[1], _s_tests[i].cmd)) {
        _s_tests[i].test();

        found = 1;
        break;
      }
    }

    if (!found)
      sl_help();
  }

  return 0;
}

