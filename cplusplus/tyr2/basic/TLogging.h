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
#ifndef __TYR_BASIC_LOGGING_HEADER_H__
#define __TYR_BASIC_LOGGING_HEADER_H__

#include <memory>
#include "TTimestamp.h"
#include "TLogStream.h"

namespace tyr { namespace basic {

class Timezone;

enum class LoggingLevel {
  LOGGINGLEVEL_TRACE = 0,
  LOGGINGLEVEL_DEBUG,
  LOGGINGLEVEL_INFO,
  LOGGINGLEVEL_WARN,
  LOGGINGLEVEL_ERROR,
  LOGGINGLEVEL_FATAL,

  COUNT_LEVELS,
};

class Logger {
public:
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
  class LoggerImpl;

  std::unique_ptr<LoggerImpl> impl_;
public:
  Logger(SourceFile file, int lineno);
  Logger(SourceFile file, int lineno, LoggingLevel level);
  Logger(SourceFile file, int lineno, LoggingLevel level, const char* func);
  Logger(SourceFile file, int lineno, bool to_abort);
  ~Logger(void);

  LogStream& stream(void);

  typedef void (*OutputCallback)(const char* msg, int len);
  typedef void (*FlushCallback)(void);
  static LoggingLevel log_level(void);
  static void set_log_level(LoggingLevel level);
  static void set_output(OutputCallback cb);
  static void set_flush(FlushCallback cb);
  static void set_timezone(const Timezone& tz);
};

const char* strerror_tl(int saved_errno);

#define TCHECK_NOTNULL(v)\
  tyr::basic::check_not_null(__FILE__, __LINE__, "'" #v "' Must be not null", (val))

template <typename T>
T* check_not_null(Logger::SourceFile file, int lineno, const char* names, T* p) {
  if (nullptr == p)
    Logger(file, lineno, LoggingLevel::LOGGINGLEVEL_FATAL).stream() << names;
  return p;
}

}}

#define TL_TRACE if (tyr::basic::Logger::log_level() <= tyr::basic::LoggingLevel::LOGGINGLEVEL_TRACE)\
  tyr::basic::Logger(__FILE__, __LINE__, tyr::basic::LoggingLevel::LOGGINGLEVEL_TRACE, __func__).stream()
#define TL_DEBUG if (tyr::basic::Logger::log_level() <= tyr::basic::LoggingLevel::LOGGINGLEVEL_DEBUG)\
  tyr::basic::Logger(__FILE__, __LINE__, tyr::basic::LoggingLevel::LOGGINGLEVEL_DEBUG, __func__).stream()
#define TL_INFO if (tyr::basic::Logger::log_level() <= tyr::basic::LoggingLevel::LOGGINGLEVEL_INFO)\
  tyr::basic::Logger(__FILE__, __LINE__).stream()
#define TL_WARN tyr::basic::Logger(__FILE__, __LINE__, tyr::basic::LoggingLevel::LOGGINGLEVEL_WARN).stream()
#define TL_ERROR tyr::basic::Logger(__FILE__, __LINE__, tyr::basic::LoggingLevel::LOGGINGLEVEL_ERROR).stream()
#define TL_FATAL tyr::basic::Logger(__FILE__, __LINE__, tyr::basic::LoggingLevel::LOGGINGLEVEL_FATAL).stream()
#define TL_SYSERR tyr::basic::Logger(__FILE__, __LINE__, false).stream()
#define TL_SYSFATAL tyr::basic::Logger(__FILE__, __LINE__, true).stream()

#endif // __TYR_BASIC_LOGGING_HEADER_H__
