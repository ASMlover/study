// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_LOGGING_HEADER_H__
#define __TYR_LOGGING_HEADER_H__

#include "TTimestamp.h"
#include "TLogStream.h"

namespace tyr {

class TimeZone;

class Logger {
public:
  enum class LogLevel : int {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    COUNT_LEVELS,
  };

  class SourceFile {
  public:
    const char* data_;
    int size_;
  public:
    template <int N>
    SourceFile(const char (&arr)[N])
      : data_(arr)
      , size_(N - 1) {
      const char* slash = strrchr(data_, '/');
      if (nullptr != slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char* fname)
      : data_(fname) {
      const char* slash = strrchr(fname, '/');
      if (nullptr != slash)
        data_ = slash + 1;
      size_ = static_cast<int>(strlen(data_));
    }
  };
private:
  class LoggerImpl {
  public:
    Timestamp time_;
    LogStream stream_;
    Logger::LogLevel level_;
    int lineno_;
    SourceFile basename_;

    LoggerImpl(Logger::LogLevel level, int old_errno, const SourceFile& file, int lineno);
    void format_time(void);
    void finish(void);
  };
  LoggerImpl impl_;
public:
  Logger(SourceFile file, int lineno);
  Logger(SourceFile file, int lineno, LogLevel level);
  Logger(SourceFile file, int lineno, LogLevel level, const char* func);
  Logger(SourceFile file, int lineno, bool to_abort);
  ~Logger(void);

  LogStream& stream(void);

  typedef void (*OutputCallback)(const char* msg, int len);
  typedef void (*FlushCallback)(void);
  static LogLevel log_level(void);
  static void set_log_level(LogLevel level);
  static void set_output(OutputCallback cb);
  static void set_flush(FlushCallback cb);
  static void set_timezone(const TimeZone& tz);
};

};

#endif // __TYR_LOGGING_HEADER_H__
