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



typedef char* (*newlisp_eval_cb)(char*);

static newlisp_eval_cb newlisp_eval;



static int 
init_newlisp(void)
{
  HMODULE m = LoadLibrary("newlisp.dll");
  if (NULL == m)
    return 0;

  newlisp_eval = (newlisp_eval_cb)GetProcAddress(m, "newlispEvalStr");

  return (NULL != newlisp_eval ? 0 : 1);
}

int 
l_eval_str(lua_State* L)
{
  char* ret;

  if (NULL == newlisp_eval) {
    if (0 != init_newlisp())
      return 0;
  }

  ret = newlisp_eval((char*)lua_tostring(L, 1));
  fprintf(stdout, "%s\n", ret);
  newlisp_eval("(exit 0)");
  lua_pushstring(L, ret);

  return 1;
}

int 
l_eval_file(lua_State* L) 
{
  char* ret;
  char buf[64];

  if (NULL == newlisp_eval) {
    if (0 != init_newlisp())
      return 0;
  }

  sprintf(buf, "(load \"%s\")", lua_tostring(L, 1));
  ret = newlisp_eval(buf);
  fprintf(stdout, "%s\n", ret);
  newlisp_eval("(exit 0)");
  lua_pushstring(L, ret);

  return 1;
}
