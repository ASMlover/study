/*
 * Copyright (c) 2012 ASMlover. All rights reserved.
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
#ifndef __MYSQL_INTERFACE_HEADER_H__
#define __MYSQL_INTERFACE_HEADER_H__

#if defined(_WINDOWS_) || defined(_MSC_VER)
  typedef unsigned __int64  uint64_t;
#elif defined(__linux__) || defined(__GNUC__)
  typedef unsigned longlong uint64_t;
#else
# error "UnSupport this platform !!!"
#endif

extern void* mysqli_connect(const char* host, unsigned int port, 
                            const char* user, const char* passwd, 
                            const char* db, const char* charset);
extern void mysqli_close(void** link_identifier);

extern int mysqli_execute(void* link_identifier, const char* query);
extern char** mysqli_row(void* link_identifier);
extern int mysqli_fields(void* link_identifier);
extern uint64_t mysqli_rows(void* link_identifier);

#endif  /* __MYSQL_INTERFACE_HEADER_H__ */
