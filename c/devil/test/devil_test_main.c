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
#include "devil_test.h"

typedef struct devil_test_t {
  const char* command;
  void (*unit_cb)(void);
} devil_test_t;

static inline void
devil_help(void)
{
  fprintf(stdout,
      "usage: slib-test.exe [options] ...\n\n"
      "   help    show help for a given topic or a help overview\n"
      "   alloc   show test result of allocator module\n"
      "   queue   show test result of queue module\n"
      "   list    show test result of list module\n"
      "   table   show test result of table module\n"
      "   array   show test result of array module\n"
      "   mutex   show test result of mutex module\n"
      "   spin    show test result of spinlock module\n"
      "   cond    show test result of condition module\n"
      "   thread  show test result of thread module\n"
      );
}

static const devil_test_t _s_units[] = {
  {"help", devil_help},
  {"alloc", devil_test_allocator},
  {"queue", devil_test_queue},
  {"list", devil_test_list},
  {"table", devil_test_table},
  {"array", devil_test_array},
  {"mutex", devil_test_mutex},
  {"spin", devil_test_spinlock},
  {"cond", devil_test_condition},
  {"thread", devil_test_thread},
};

int
main(int argc, char* argv[])
{
  if (argc < 2) {
    devil_help();
  }
  else {
    int i, found = 0;
    int len = countof(_s_units);
    for (i = 0; i < len; ++i) {
      if (cmdeq(argv[1], _s_units[i].command)) {
        _s_units[i].unit_cb();

        found = 1;
        break;
      }
    }

    if (!found)
      devil_help();
  }

  return 0;
}

