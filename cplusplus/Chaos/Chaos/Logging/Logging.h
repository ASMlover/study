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
#ifndef CHAOS_LOGGING_LOGGING_H
#define CHAOS_LOGGING_LOGGING_H

#include <memory>
#include <Chaos/UnCopyable.h>
#include <Chaos/Datetime/Timestamp.h>
#include <Chaos/Logging/LogStream.h>

namespace Chaos {

class Timezone;

enum class LoggingLevel {
  LOGGINGLEVEL_TRACE = 0,
  LOGGINGLEVEL_DEBUG,
  LOGGINGLEVEL_INFO,
  LOGGINGLEVEL_WARN,
  LOGGINGLEVEL_ERROR,
  LOGGINGLEVEL_FATAL,

  LOGGINGLEVEL_COUNTS,
};

class Logger : private UnCopyable {
  class LoggerImpl;

  std::unique_ptr<LoggerImpl> impl_;
public:
  class SourceFile {
  public:
    const char* data_;
    size_t size_;

    template <size_t N>
    SourceFile(const char (&arr)[N])
      : data_(arr)
      , size_(N - 1) {
      const char* slash = strrchr(data_, '/');
      if (nullptr != slash) {
        data_ = slash + 1;
        size_ -= static_cast<size_t>(data_ - arr);
      }
    }

    explicit SourceFile(const char* fname)
      : data_(fname) {
      const char* slash = strrchr(fname, '/');
      if (nullptr != slash)
        data_ = slash + 1;
      size_ = strlen(data_);
    }
  };

  typedef void (*OutputCallback)(const char* buf, size_t len);
  typedef void (*FlushCallback)(void);
public:
  Logger(SourceFile file, int lineno);
  Logger(SourceFile file, int lineno, LoggingLevel level);
  Logger(SourceFile file, int lineno, LoggingLevel level, const char* func);
  Logger(SourceFile file, int lineno, bool do_abort);
  ~Logger(void);

  LogStream& get_stream(void);

  static LoggingLevel get_loglevel(void);
  static void set_loglevel(LoggingLevel level);
  static void set_output_callback(OutputCallback cb);
  static void set_flush_callback(FlushCallback cb);
  static void set_timezone(const Timezone& tz);
};

const char* strerror_tl(int saved_errno);

template <typename T>
inline T* check_non_nil(Logger::SourceFile file, int lineno, const char* names, T* p) {
  if (nullptr == p)
    Logger(file, lineno, LoggingLevel::LOGGINGLEVEL_FATAL).get_stream() << names;
  return p;
}

#if !defined(CHECK_NONIL)
# define CHECK_NONIL(value)\
  Chaos::check_non_nil(__FILE__, __LINE__, "`" #value "` must be non-nil", (value));
#endif

}

#define CHAOSLOG_TRACE if (Chaos::Logger::get_loglevel() <= Chaos::LoggingLevel::LOGGINGLEVEL_TRACE)\
  Chaos::Logger(__FILE__, __LINE__, Chaos::LoggingLevel::LOGGINGLEVEL_TRACE, __func__).get_stream()
#define CHAOSLOG_DEBUG if (Chaos::Logger::get_loglevel() <= Chaos::LoggingLevel::LOGGINGLEVEL_DEBUG)\
  Chaos::Logger(__FILE__, __LINE__, Chaos::LoggingLevel::LOGGINGLEVEL_DEBUG, __func__).get_stream()
#define CHAOSLOG_INFO if (Chaos::Logger::get_loglevel() <= Chaos::LoggingLevel::LOGGINGLEVEL_INFO)\
  Chaos::Logger(__FILE__, __LINE__).get_stream()
#define CHAOSLOG_WARN Chaos::Logger(__FILE__, __LINE__, Chaos::LoggingLevel::LOGGINGLEVEL_WARN).get_stream()
#define CHAOSLOG_ERROR Chaos::Logger(__FILE__, __LINE__, Chaos::LoggingLevel::LOGGINGLEVEL_ERROR).get_stream()
#define CHAOSLOG_FATAL Chaos::Logger(__FILE__, __LINE__, Chaos::LoggingLevel::LOGGINGLEVEL_FATAL).get_stream()
#define CHAOSLOG_SYSERR Chaos::Logger(__FILE__, __LINE__, false).get_stream()
#define CHAOSLOG_SYSFATAL Chaos::Logger(__FILE__, __LINE__, true).get_stream()

#endif // CHAOS_LOGGING_LOGGING_H
