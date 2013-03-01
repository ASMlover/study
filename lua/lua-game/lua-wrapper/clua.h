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
#ifndef __C_WRAPPER_LUA_HEADER_H__
#define __C_WRAPPER_LUA_HEADER_H__ 

typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State* L);

typedef struct clua_s {
  lua_State* L;
  void (*error_handler)(const char* error);
} clua_t;


extern int clua_init(clua_t* clua);
extern void clua_destroy(clua_t* clua);
extern int clua_run_script(clua_t* clua, const char* script_file);
extern int clua_run_string(clua_t* clua, const char* script_string);
extern void clua_register_function(clua_t* clua, 
    const char* function_name, lua_CFunction function);
extern const char* clua_get_string_arg(clua_t* clua, 
    int narg, const char* def_string);
extern double clua_get_number_arg(clua_t* clua, 
    int narg, double def_number);
extern void clua_push_string(clua_t* clua, const char* string);
extern void clua_push_number(clua_t* clua, double number);
extern const char* clua_strerror(clua_t* clua);
extern lua_State* clua_get_context(clua_t* clua);

#endif  /* __C_WRAPPER_LUA_HEADER_H__ */
