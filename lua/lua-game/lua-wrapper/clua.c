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
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "clua.h"



int 
clua_init(clua_t* clua)
{
  clua->L = luaL_newstate();
  if (NULL == clua->L)
    return -1;
  clua->error_handler = NULL;

  luaL_openlibs(clua->L);

  return 0;
}

void 
clua_destroy(clua_t* clua)
{
  if (NULL != clua->L)
    lua_close(clua->L);
  clua->L = NULL;
}

int 
clua_run_script(clua_t* clua, const char* script_file)
{
  if (0 != luaL_loadfile(clua->L, script_file)) {
    if (NULL != clua->error_handler) {
      char buf[256];
      sprintf(buf, "Lua Error: load script-%s\nError Message-%s\n", 
        script_file, luaL_checkstring(clua->L, -1));
      clua->error_handler(buf);
    }

    return -1;
  }

  if (0 != lua_pcall(clua->L, 0, LUA_MULTRET, 0)) {
    if (NULL != clua->error_handler) {
      char buf[256];
      sprintf(buf, "Lua Error: run script-%s\nError Message-%s\n", 
        script_file, luaL_checkstring(clua->L, -1));
      clua->error_handler(buf);
    }

    return -1;
  }

  return 0;
}

int 
clua_run_string(clua_t* clua, const char* script_string)
{
  if (0 != luaL_loadbuffer(clua->L, 
    script_string, strlen(script_string), NULL)) {
    if (NULL != clua->error_handler) {
      char buf[256];
      sprintf(buf, "Lua Error: load script-%s\nError Message-%s\n", 
        script_string, luaL_checkstring(clua->L, -1));
      clua->error_handler(buf);
    }
    
    return -1;
  }

  if (0 != lua_pcall(clua->L, 0, LUA_MULTRET, 0)) {
    if (NULL != clua->error_handler) {
      char buf[256];
      sprintf(buf, "Lua Error: run script-%s\nError Message-%s\n", 
        script_string, luaL_checkstring(clua->L, -1));
      clua->error_handler(buf);
    }

    return -1;
  }

  return 0;
}

void 
clua_register_function(clua_t* clua, 
    const char* function_name, lua_CFunction function)
{
  lua_register(clua->L, function_name, function);
}

const char* 
clua_get_string_arg(clua_t* clua, int narg, const char* def_string) 
{
  return luaL_optstring(clua->L, narg, def_string);
}

double 
clua_get_number_arg(clua_t* clua, int narg, double def_number)
{
  return luaL_optnumber(clua->L, narg, def_number);
}

void 
clua_push_string(clua_t* clua, const char* string)
{
  lua_pushstring(clua->L, string);
}

void 
clua_push_number(clua_t* clua, double number)
{
  lua_pushnumber(clua->L, number);
}

const char* 
clua_strerror(clua_t* clua)
{
  return luaL_checkstring(clua->L, -1);
}

lua_State* 
clua_get_context(clua_t* clua)
{
  return clua->L;
}
