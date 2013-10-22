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
#include <stdarg.h>
#include <string.h>
#include "common.h"



bool CommonLib::loaded_ = false;
CommonLib::CommonLib(void)
{
}

CommonLib::~CommonLib(void)
{
}

CommonLib& 
CommonLib::Singleton(void)
{
  static CommonLib s_comm_lib;

  return s_comm_lib;
}

void 
CommonLib::Init(void)
{
  if (!loaded_) {
    WSADATA wd;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wd))
      LOG_FAIL("WSAStartup failed err-code(%d)\n", NetErrno());

    loaded_ = true;
  }
}

void 
CommonLib::Destroy(void)
{
  if (loaded_)
    WSACleanup();
}



void 
CommonLog(int severity, const char* file, int line, const char* format, ...)
{
  static char buffer[10240];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  FILE* stream = stdout;
  switch (severity) {
  case kLoggingModeDebug:
  case kLoggingModeMessage:
  case kLoggingModeWarning:
    stream = stdout;
    break;
  case kLoggingModeError:
  case kLoggingModeFailed:
    stream = stderr;
    break;
  }

  fprintf(stream, "%s", buffer);
  if (kLoggingModeFailed == severity)
    abort();
}
