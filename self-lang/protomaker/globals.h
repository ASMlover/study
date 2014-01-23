/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __GLOBALS_HEADER_H__
#define __GLOBALS_HEADER_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Boolean {
  BOOL_NO = 0, 
  BOOL_YES, 
};

enum TokenType {
  TOKEN_TYPE_EOF = 0,       /* EOF */
  TOKEN_TYPE_ERR, 

  TOKEN_TYPE_DEFAULT,       /* default */
  TOKEN_TYPE_DEFINE,        /* define */
  TOKEN_TYPE_ENUM,          /* enum */
  TOKEN_TYPE_MESSAGE,       /* message */
  TOKEN_TYPE_PROTOCOL,      /* protocol */
  TOKEN_TYPE_TYPE,          /* type */

  TOKEN_TYPE_CHAR,          /* char */
  TOKEN_TYPE_BYTE,          /* byte */
  TOKEN_TYPE_INT8,          /* int8 */
  TOKEN_TYPE_UINT8,         /* uint8 */
  TOKEN_TYPE_INT16,         /* int16 */
  TOKEN_TYPE_UINT16,        /* uint16 */
  TOKEN_TYPE_INT32,         /* int32 */
  TOKEN_TYPE_UINT32,        /* uint32 */
  TOKEN_TYPE_INT64,         /* int64 */
  TOKEN_TYPE_UINT64,        /* uint64 */
  TOKEN_TYPE_REAL32,        /* real32 */
  TOKEN_TYPE_REAL64,        /* real64 */

  TOKEN_TYPE_ASSIGN,        /* = */
  TOKEN_TYPE_INHERIT,       /* < */
  TOKEN_TYPE_ACCESS,        /* . */
  TOKEN_TYPE_LBRACKET,      /* [ */
  TOKEN_TYPE_RBRACKET,      /* ] */
  TOKEN_TYPE_LBRACE,        /* { */
  TOKEN_TYPE_RBRACE,        /* } */

  TOKEN_TYPE_CINT,          /* const int */
  TOKEN_TYPE_CREAL,         /* const real */
  TOKEN_TYPE_ID, 

  TOKEN_TYPE_COMMENT,       /* # */
};


#define MAX_TOKEN   (256)

#if !defined(countof)
# define countof(x)     ((sizeof(x)) / (sizeof(*x)))
#endif



extern FILE*  g_source_stream;
extern FILE*  g_scan_stream;
extern int    g_line_numer;
extern char   g_token[MAX_TOKEN];


#endif  //! __GLOBALS_HEADER_H__
