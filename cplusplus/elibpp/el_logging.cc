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
#if defined(_WINDOWS_) || defined(_MSC_VER)
# include <windows.h>
# include <io.h>
# include <direct.h>
#elif defined(__linux__)
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>
#endif
#include <string.h>
#include <stdarg.h>
#include "el_time.h"
#include "el_logging.h"


#define ROOT_DIR  ("logging")

namespace el {

static inline int 
logging_mkdir(const char* path)
{
#if defined(_WINDOWS_) || defined(_MSC_VER)
  return mkdir(path);
#elif defined(_MSC_VER)
  return mkdir(path, S_IRWXU);
#endif
}

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
    logging_mkdir(ROOT_DIR);

  char path[MAX_PATH];
  sprintf(path, "%s/%s", ROOT_DIR, directory);
  if (0 != access(path, 0))
    logging_mkdir(path);
}



Logging::Logging(void)
{
  memset(file_list_, 0, sizeof(file_list_));
}

Logging::~Logging(void)
{
  for (int i = 0; i < ST_COUNT; ++i) {
    if (NULL != file_list_[i].stream)
      fclose(file_list_[i].stream);
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
  case ST_MESSGAE:
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
Logging::GetFileStream(int severity, Time* time)
{
  if ((severity < 0 || severity >= ST_COUNT) || NULL == time)
    return NULL;

  FILE* stream;
  const char* directory = GetSeverityName(severity);
  if (NULL == file_list_[severity].stream) {
    CreateLogDirectory(directory);

    char fname[MAX_PATH];
    sprintf(fname, "%s/%s/%04d%02d%02d.log", 
        ROOT_DIR, directory, time->year, time->mon, time->day);
    stream = fopen(fname, "a+");
    setvbuf(stream, NULL, _IOFBF, DEF_BUFSIZE);

    file_list_[severity].year = time->year;
    file_list_[severity].mon  = time->mon;
    file_list_[severity].day  = time->day;
    file_list_[severity].stream = stream;
  }
  else {
    if (KeyEqual(&file_list_[severity], time)) {
      stream = file_list_[severity].stream;
    }
    else {
      fclose(file_list_[severity].stream);

      char fname[MAX_PATH];
      sprintf(fname, "%s/%s/%04d%02d%02d.log", 
          ROOT_DIR, directory, time->year, time->mon, time->day);
      stream = fopen(fname, "a+");
      setvbuf(stream, NULL, _IOFBF, DEF_BUFSIZE);

      file_list_[severity].year = time->year;
      file_list_[severity].mon  = time->mon;
      file_list_[severity].day  = time->day;
      file_list_[severity].stream = stream;
    }
  }

  return stream;
}


void 
Logging::Write(int severity, const char* format, ...)
{
  Time time;
  Localtime(&time);

  FILE* stream = GetFileStream(severity, &time);
  if (NULL == stream)
    return;

  va_list ap;
  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);
}

void 
Logging::WriteX(int severity, char* file, int line, const char* format, ...)
{
  Time time;
  Localtime(&time);

  FILE* stream = GetFileStream(severity, &time);
  if (NULL == stream)
    return;

  va_list ap;
  va_start(ap, format);

  char buf[1024];
  vsprintf(buf, format, ap);
  fprintf(stream, "[%02d:%02d:%02d:%03d] [%s](%d) : %s", 
      time.hour, time.min, time.sec, time.millitm, file, line, buf);

  va_end(ap);
}


}
