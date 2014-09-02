// Copyright (c) 2014 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include "eutil.h"
#include "el_time.h"
#include "el_logging.h"

#define ROOT_DIR  ("logging")

namespace el {

static inline int logging_mkdir(const char* path) {
  int ret = -1;
#if defined(EUTIL_WIN)
  ret = mkdir(path);
#elif defined(EUTIL_LINUX)
  int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
  ret = mkdir(path, mode);
#endif

  return ret;
}

static inline void CreateLogDirectory(const char* directory) {
  if (0 == access(ROOT_DIR, 0))
    logging_mkdir(ROOT_DIR);

  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "%s/%s", ROOT_DIR, directory);
  if (0 != access(path, 0))
    logging_mkdir(path);
}

Logging::Logging(void) {
  files_[SeverityType::SEVERITYTYPE_DEBUG] = File();
  files_[SeverityType::SEVERITYTYPE_MESSAGE] = File();
  files_[SeverityType::SEVERITYTYPE_WARNING] = File();
  files_[SeverityType::SEVERITYTYPE_ERROR] = File();
  files_[SeverityType::SEVERITYTYPE_FAIL] = File();
}

const char* Logging::GetSeverityName(SeverityType severity) {
  switch (severity) {
  case SeverityType::SEVERITYTYPE_DEBUG:
    return "debug";
  case SeverityType::SEVERITYTYPE_MESSAGE:
    return "message";
  case SeverityType::SEVERITYTYPE_WARNING:
    return "warning";
  case SeverityType::SEVERITYTYPE_ERROR:
    return "error";
  case SeverityType::SEVERITYTYPE_FAIL:
    return "fail";
  }

  return "???";
}

FILE* Logging::GetFileStream(SeverityType severity, const Time& time) {
  if ((severity < SeverityType::SEVERITYTYPE_DEBUG) 
      || (severity > SeverityType::SEVERITYTYPE_FAIL))
    return nullptr;

  FILE* stream;
  const char* directory = GetSeverityName(severity);
  if (nullptr == files_[severity].stream) {
    CreateLogDirectory(directory);

    char fname[MAX_PATH];
    snprintf(fname, MAX_PATH, "./%s/%s/%04d%02d%02d.log", 
        ROOT_DIR, directory, time.year, time.mon, time.day);
    stream = fopen(fname, "a+");
    setvbuf(stream, nullptr, _IOFBF, DEF_BUFSIZE);

    files_[severity].year = time.year;
    files_[severity].mon  = time.mon;
    files_[severity].day  = time.day;
    files_[severity].stream = stream;
  }
  else {
    if (files_[severity] == time) {
      stream = files_[severity].stream;
    }
    else {
      fclose(files_[severity].stream);

      char fname[MAX_PATH];
      snprintf(fname, MAX_PATH, "./%s/%s/%04d%02d%02d.log", 
          ROOT_DIR, directory, time.year, time.mon, time.day);
      stream = fopen(fname, "a+");
      setvbuf(stream, nullptr, _IOFBF, DEF_BUFSIZE);

      files_[severity].year = time.year;
      files_[severity].mon  = time.mon;
      files_[severity].day  = time.day;
      files_[severity].stream = stream;
    }
  }
  
  return stream;
}

void Logging::Write(SeverityType severity, const char* format, ...) {
  Time time;
  GetTime(time);

  FILE* stream = GetFileStream(severity, time);
  if (nullptr == stream)
    return;

  va_list ap;
  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);
}

void Logging::WriteX(SeverityType severity, 
    const char* file, int line, const char* format, ...) {
  Time time;
  GetTime(time);

  FILE* stream = GetFileStream(severity, time);
  if (nullptr == stream)
    return;

  char buf[1024];
  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  fprintf(stream, "[%02d:%02d:%02d:%03d] [%s](%d) : %s", 
      time.hour, time.min, time.sec, time.millitm, file, line, buf);
}

}
