/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#ifndef Nj_NJCONFIG_H
#define Nj_NJCONFIG_H

#include <stddef.h>
#include <stdint.h>

typedef char          Nj_char_t;
typedef unsigned char Nj_uchar_t;
typedef int8_t        Nj_int8_t;
typedef uint8_t       Nj_uint8_t;
typedef int16_t       Nj_int16_t;
typedef uint16_t      Nj_uint16_t;
typedef int32_t       Nj_int32_t;
typedef uint32_t      Nj_uint32_t;
typedef int64_t       Nj_int64_t;
typedef uint64_t      Nj_uint64_t;
typedef intptr_t      Nj_intptr_t;
typedef uintptr_t     Nj_uintptr_t;
typedef int           Nj_int_t;
typedef size_t        Nj_size_t;
#if defined(HAVE_SSIZE_T)
  typedef ssize_t     Nj_ssize_t;
#else
  typedef Nj_intptr_t Nj_ssize_t;
#endif
typedef enum _bool {FALSE, TRUE} Nj_bool_t;

#if !defined(NjAPI_FUNC)
# define NjAPI_FUNC(RTYPE) extern RTYPE
#endif
#if !defined(NjAPI_DATA)
# define NjAPI_DATA(RTYPE) extern RTYPE
#endif

#define Nj_UNUSED(x) ((void)x)

#if !defined(Nj_CHECK)
# include "njlog.h"
# define Nj_CHECK(cond, msg) do {\
    if (!(cond))\
      njlog_fatal("%s\n", msg);\
  } while (0)
#endif

#endif /* Nj_NJCONFIG_H */
