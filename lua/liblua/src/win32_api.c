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
#include <windows.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>




typedef void* (__stdcall *func_call)();
 
static int api_call(lua_State *L)
{
  int i, type;
  int n = lua_gettop(L);
  func_call fc = (func_call)lua_touserdata(L, lua_upvalueindex(2));
  void* ret;
  do {
    void** arg = (void**)_alloca(n * sizeof(void*));
    for (i = 0; i < n; ++i) {
      type = lua_type(L, i + 1);
      switch (type) {
      case LUA_TNIL:
        arg[i] = 0;
        break;
      case LUA_TNUMBER:
        arg[i] = (void*)lua_tointeger(L, i + 1);
        break;
      case LUA_TBOOLEAN:
        arg[i] = (void*)lua_toboolean(L, i + 1);
        break;
      case LUA_TSTRING:
        arg[i] = (void*)lua_tostring(L, i + 1);
        break;
      case LUA_TLIGHTUSERDATA:
        arg[i] = lua_touserdata(L, i + 1);
        break;
      default:
        lua_pushstring(L, "unknown argument type");
        lua_error(L);
        break;
      }
    }
    ret = fc();
  } while (0);
  switch (lua_type(L, lua_upvalueindex(1))) {
  case LUA_TNIL:
    lua_pushlightuserdata(L, ret);
    break;
  case LUA_TNUMBER:
    lua_pushinteger(L, (int)ret);
    break;
  case LUA_TBOOLEAN:
    lua_pushboolean(L, (int)ret);
    break;
  case LUA_TSTRING:
    lua_pushstring(L, (const char*)ret);
    break;
  default:
    lua_pushstring(L, "unknown return value type");
    lua_error(L);
    break;
  }

  return 1;
}


int 
ll_open_dll(lua_State* L)
{
  const char* dllname = lua_tostring(L, 1);
  HMODULE h = LoadLibrary(dllname);
  if (NULL == h)
    return 0;
  
  lua_pushlightuserdata(L, h);
  return 1;
}

int 
ll_close_dll(lua_State* L)
{
  HMODULE h = (HMODULE)lua_touserdata(L, 1);

  FreeLibrary(h);
  return 0;
}

int 
ll_get_function(lua_State* L)
{
  HMODULE h = (HMODULE)lua_touserdata(L, 1);
  const char* funcname = lua_tostring(L, 2);
  void* func = GetProcAddress(h, funcname);

  if (NULL == func)
    return 0;

  lua_pushvalue(L, 3);
  lua_pushlightuserdata(L, func);
  lua_pushcclosure(L, api_call, 2);

  return 1;
}
