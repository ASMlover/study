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
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "logging.h"




static char _s_buffer[10240] = {0};

static inline const char* 
GetFname(int severity)
{
  switch (severity) {
  case Logging::SEVERITY_DEBUG:
    return "debug.log";
  case Logging::SEVERITY_MESSAGE:
    return "msg.log";
  case Logging::SEVERITY_WARNING:
    return "warn.log";
  case Logging::SEVERITY_ERROR:
    return "error.log";
  }

  return "???.log";
}


Logging::Logging(void)
{
}

Logging::~Logging(void)
{
  std::map<int, FILE*>::iterator it;
  for (it = file_list_.begin(); it != file_list_.end(); ++it) 
    fclose(it->second);
  file_list_.clear();
}

Logging& 
Logging::Singleton(void)
{
  static Logging s;
  return s;
}

int 
Logging::Write(int severity, const char* file, 
    const char* function, int line, const char* format, ...)
{
  FILE* stream;

  std::map<int, FILE*>::iterator it = file_list_.find(severity);
  if (it == file_list_.end()) {
    const char* fname = GetFname(severity);

    stream = fopen(fname, "a+");
    assert(NULL != stream);
    setvbuf(stream, NULL, _IOFBF, DEF_BUFSIZE);

    file_list_[severity] = stream;
  }
  else {
    stream = it->second;
  }

  va_list ap;
  va_start(ap, format);
  vsprintf(_s_buffer, format, ap);
  va_end(ap);

  return fprintf(stream, "%s - %s (%d) : %s", 
      file, function, line, _s_buffer);
}
