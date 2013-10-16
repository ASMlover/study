//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef _WINDOWS_
# include <winsock2.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "global.h"


WSLib::WSLib(void)
{
  WSADATA wd;

  if (0 != WSAStartup(MAKEWORD(2, 2), &wd))
    LOG_FAIL("WSAStartup failed err-code (%d)\n", WSAGetLastError());
}

WSLib::~WSLib(void)
{
  WSACleanup();
}



void 
LogWrite(int severity, const char* file, int line, const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  char buf[1024];
  vsprintf(buf, format, ap);
  va_end(ap);

  FILE* stream;
  switch (severity) {
  case LT_DEBUG:
    stream = stdout;
    break;
  case LT_ERROR:
  case LT_FAIL:
    stream = stderr;
    break;
  default:
    stream = stdout;
  }
  
  fprintf(stream, "[%s] [%d] : %s", file, line, buf);
  if (LT_FAIL == severity)
    abort();
}
