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
  if (0 != access(ROOT_DIR, 0))
    logging_mkdir(ROOT_DIR);

  char path[MAX_PATH];
  snprintf(path, MAX_PATH, "%s/%s", ROOT_DIR, directory);
  if (0 != access(path, 0))
    logging_mkdir(path);
}

class LogFile : private UnCopyable {
  typedef std::shared_ptr<FILE> FilePtr;

  FilePtr file_;
  Date    date_;
  public:
  LogFile(void)
    : file_(FilePtr(nullptr)) {
    memset(&date_, 0, sizeof(date_));
  }

  ~LogFile(void) {
  }

  inline bool operator==(const Date& date) const {
    return (date_.year == date.year 
        && date_.mon == date.mon
        && date_.day == date.day);
  }

  inline FilePtr& file(void) {
    return file_;
  }

  inline FILE* Set(const Date& date, const char* fname) {
    file_.reset(fopen(fname, "a+"), fclose);
    if (!file_)
      return nullptr;

    setvbuf(file_.get(), nullptr, _IOFBF, 16*1024);
    date_ = date;

    return file_.get();
  }
};

Logging::Logging(void) {
  files_.insert(std::make_pair(
        SeverityType::SEVERITYTYPE_DEBUG, LogFilePtr(new LogFile())));
  files_.insert(std::make_pair(
        SeverityType::SEVERITYTYPE_MESSAGE, LogFilePtr(new LogFile())));
  files_.insert(std::make_pair(
        SeverityType::SEVERITYTYPE_WARNING, LogFilePtr(new LogFile())));
  files_.insert(std::make_pair(
        SeverityType::SEVERITYTYPE_ERROR, LogFilePtr(new LogFile())));
  files_.insert(std::make_pair(
        SeverityType::SEVERITYTYPE_FAIL, LogFilePtr(new LogFile())));
}

Logging::~Logging(void) {
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

FILE* Logging::GetStream(SeverityType severity) {
  auto file = files_[severity];
  if (!file)
    return nullptr;

  FILE* stream = nullptr;
  Date date;
  GetDate(date);
  if (file->file() && *file == date) {
    stream = file->file().get();
  }
  else {
    const char* directory = GetSeverityName(severity);
    CreateLogDirectory(directory);

    char fname[MAX_PATH];
    snprintf(fname, MAX_PATH, "./%s/%s/%04d%02d%02d.log", 
        ROOT_DIR, directory, date.year, date.mon, date.day);
    stream = file->Set(date, fname);
  }

  return stream;
}

void Logging::Write(SeverityType severity, const char* format, ...) {
  Time time;
  GetTime(time);

  FILE* stream = GetStream(severity);
  if (nullptr == stream)
    return;
  fprintf(stream, "[%02d:%02d:%02d.%03d] ", 
      time.hour, time.min, time.sec, time.millitm);

  va_list ap;
  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);
}

void Logging::WriteX(SeverityType severity, 
    const char* file, int line, const char* format, ...) {
  Time time;
  GetTime(time);

  FILE* stream = GetStream(severity);
  if (nullptr == stream)
    return;
  fprintf(stream, "[%02d:%02d:%02d.%03d] %s(%d): ", 
      time.hour, time.min, time.sec, time.millitm, file, line);

  va_list ap;
  va_start(ap, format);
  vfprintf(stream, format, ap);
  va_end(ap);
}

}
