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
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include "logging.h"



#define ROOT_DIR    ("logging")



static inline bool 
KeyEqual(LogFile* lf, Time* t)
{
  return (lf->year == t->year 
      && lf->mon == t->mon 
      && lf->day == t->day);
}

static inline void 
CreateLogDirectory(const char* directory) 
{
  if (0 != access(ROOT_DIR, 0))
    mkdir(ROOT_DIR);

  char path[MAX_PATH];
  sprintf(path, "%s/%s", ROOT_DIR, directory);
  if (0 != access(path, 0))
    mkdir(path);
}




Logging::Logging(void)
{
}

Logging::~Logging(void)
{
  std::map<int, LogFile>::iterator it;
  for (it = file_list_.begin(); it != file_list_.end(); ++it) {
    if (NULL != it->second.stream)
      fclose(it->second.stream);
  }
}


Logging& 
Logging::Singleton(void)
{
  static Logging _s_logging;
  return _s_logging;
}

const char* 
Logging::GetSeverityName(int severity)
{
  switch (severity) {
  case ST_DEBUG:
    return "debug";
  case ST_MESSAGE:
    return "message";
  case ST_WARNING:
    return "warn";
  case ST_ERROR:
    return "error";
  case ST_FAIL:
    return "fail";
  }

  return "???";
}

FILE* 
Logging::GetFileStream(int severity)
{
  Time time;
  Localtime(&time);

  FILE* stream;
  const char* dir = GetSeverityName(severity);
  std::map<int, LogFile>::iterator it = file_list_.find(severity);
  if (it == file_list_.end()) {
    CreateLogDirectory(dir);

    char fname[MAX_PATH];
    sprintf(fname, "%s/%s/%04d%02d%02d.log", 
        ROOT_DIR, dir, time.year, time.mon, time.day);
    stream = fopen(fname, "a+");

    LogFile lf;
    lf.year = time.year;
    lf.mon  = time.mon;
    lf.day  = time.day;
    lf.stream = stream;
    file_list_[severity] = lf;
  }
  else {
    if (KeyEqual(&it->second, &time)) {
      stream = it->second.stream;
    }
    else {
      if (NULL != it->second.stream)
        fclose(it->second.stream);

      char fname[MAX_PATH];
      sprintf(fname, "%s/%s/%04d%02d%02d.log", 
          ROOT_DIR, dir, time.year, time.mon, time.day);
      stream = it->second.stream = fopen(fname, "a+");
      it->second.year = time.year;
      it->second.mon  = time.mon;
      it->second.day  = time.day;
    }
  }

  return stream;
}

void 
Logging::Write(int severity, const char* format, ...)
{
  FILE* stream = GetFileStream(severity);
  if (NULL == stream)
    return;

  va_list ap;
  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);
}
