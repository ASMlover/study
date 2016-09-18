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
#include <string.h>

#include "common.h"
#include "futil.h"
#include "global.h"
#include "parser.h"

static void
_Usage(int err)
{
  const char* str = "usage: barry [-hv] <file>";
  if (err)
    ERROR("%s\n", str);
  else
    ERROR("%s\n", str);
}

int
main(int argc, char* argv[]) {
  const char* file = NULL;
  const char* src = NULL;

  if (argc < 2) {
    _Usage(1);
    return 1;
  }

  (void)*argv++;
  FOREACH(argv, char* opt, {
      if ('-' == opt[0]) {
        if (EQUAL(opt, "-h") || EQUAL(opt, "--help")) {
          _Usage(0);
          exit(0);
        }
        else if (EQUAL(opt, "-v") || EQUAL(opt, "--version")) {
          ECHO("barry v%s\n", BARRY_VERSION);
          exit(0);
        }
        else {
          ERROR("error: unknown option `%s`\n", opt);
          _Usage(1);
          exit(1);
        }
        continue;
      }

      file = opt;
      break;
  });

  if (0 != fu_Exists(file)) {
    ERROR("Error: cannot find the file `%s`\n", file);
    return 1;
  }

  src = fu_Read(file);
  if (NULL == src) {
    ERROR("Error: error reading `%s`\n", file);
    return 1;
  }

  barry_InitGlobals();
  return barry_Parse(file, src, NULL);
}
