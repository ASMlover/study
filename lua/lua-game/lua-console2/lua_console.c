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
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
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
#include <clua.h>



static int 
l_version(lua_State* L)
{
  fprintf(stdout, "Version 2.0 of lua-console\n");
  return 0;
}

static char* 
get_command(void)
{
  static char cmd_buf[256];

  fprintf(stdout, "lua-console> ");
  return gets(cmd_buf);
}

int 
main(int argc, char* argv[])
{
  clua_t clua;
  char* cmd;
  fprintf(stdout, "Enter lua commands at the prompt, [quit] to exit\n\n");

  clua_init(&clua);
  clua_register_function(&clua, "version", l_version);

  cmd = get_command();
  while (0 != stricmp("quit", cmd)) {
    if (0 != clua_run_string(&clua, cmd)) 
      fprintf(stderr, "Error: %s\n", clua_strerror(&clua));

    cmd = get_command();
  }

  clua_destroy(&clua);

  return 0;
}
