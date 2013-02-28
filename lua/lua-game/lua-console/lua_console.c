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
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>



static int 
L_Version(lua_State* L)
{
  fputs("This is version 1.o of the lua-console", stdout);
  fputs(LUA_VERSION, stdout);
  fputs(LUA_COPYRIGHT, stdout);
  fputs(LUA_AUTHORS, stdout);

  return 0;
}


static luaL_reg s_console_glue[] = {
  {"Version", L_Version}, 
  {NULL, NULL}, 
};

static char s_buffer[256];


static char* 
get_command(void) 
{
  fprintf(stdout, "Ready> ");
  gets(s_buffer);
  fprintf(stdout, "\n");

  return s_buffer;
}


int 
main(int argc, char* argv[])
{
  lua_State* L;
  int i;
  char* cmd;

  fputs("Enter lua command at the prompt, [quit] to exit\n", stdout);

  L = lua_open();
  luaL_openlibs(L);

  fputs("lua init success ...", stdout);
  for (i = 0; NULL != s_console_glue[i].name; ++i)
    lua_register(L, s_console_glue[i].name, s_console_glue[i].func);

  cmd = get_command();
  while (0 != stricmp("quit", cmd)) {
    if (0 == luaL_loadbuffer(L, cmd, strlen(cmd), NULL)) {
      if (0 != lua_pcall(L, 0, LUA_MULTRET, 0)) 
        fprintf(stdout, "Error: %s\n", luaL_checkstring(L, -1));
    }
    else 
      fprintf(stdout, "Error: %s\n", luaL_checkstring(L, -1)); 

    cmd = get_command();
  }

  lua_close(L);

  return 0;
}
