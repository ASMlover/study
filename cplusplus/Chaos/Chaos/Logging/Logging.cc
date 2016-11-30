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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <Chaos/Platform.h>
#include <Chaos/Types.h>
#include <Chaos/OS/OS.h>
#include <Chaos/Datetime/Timezone.h>
#include <Chaos/Concurrent/CurrentThread.h>
#include <Chaos/Logging/Logging.h>

namespace Chaos {

#if defined(CHAOS_WINDOWS)
  __declspec(thread) char t_errbuf[512];
  __declspec(thread) char t_timebuf[32];
  __declspec(thread) time_t t_last_seconds;
#else
  __thread char t_errbuf[512];
  __thread char t_timebuf[32];
  __thread time_t t_last_seconds;
#endif

const char* strerror_tl(int saved_errno) {
#if defined(CHAOS_LINUX)
  return Chaos::kern_strerror(saved_errno, t_errbuf, sizeof(t_errbuf));
#else
  Chaos::kern_strerror(saved_errno, t_errbuf, sizeof(t_errbuf));
  return t_errbuf;
#endif
}

LoggingLevel init_loglevel(void) {
  if (getenv("CHAOS_LOG_TRACE"))
    return LoggingLevel::LOGGINGLEVEL_TRACE;
  else if (getenv("CHAOS_LOG_DEBUG"))
    return LoggingLevel::LOGGINGLEVEL_DEBUG;
  else
    return LoggingLevel::LOGGINGLEVEL_INFO;
}

LoggingLevel g_loglevel = init_loglevel();

const char* kLogLevelNames[static_cast<int>(LoggingLevel::LOGGINGLEVEL_COUNTS)] = {
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL "
};

class ValueT {
public:
  const char* str_;
  size_t len_;

  ValueT(const char* s, size_t n)
    : str_(s)
    , len_(n) {
    CHAOS_CHECK(strlen(str_) == len_, "ValueT - length of `str_` must be equal to `len_`");
  }
};

inline LogStream& operator<<(LogStream& s, ValueT v) {
  s.append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& f) {
  s.append(f.data_, f.size_);
  return s;
}

void default_logging_output(const char* buf, size_t len) {
  fwrite(buf, 1, len, stdout);
}

void default_logging_flush(void) {
  fflush(stdout);
}

Logger::OutputCallback g_output_fn = default_logging_output;
Logger::FlushCallback g_flush_fn = default_logging_flush;
Timezone g_log_timezone;

class Logger::LoggerImpl {
public:
  Timestamp time_;
  LogStream stream_;
  LoggingLevel level_;
  int lineno_;
  SourceFile basename_;

  LoggerImpl(LoggingLevel level, int saved_errno, const SourceFile& file, int lineno);
  void format_time(void);
  void finish(void);
};

Logger::LoggerImpl::LoggerImpl(LoggingLevel level, int saved_errno, const SourceFile& file, int lineno)
  : time_(Timestamp::now())
  , stream_()
  , level_(level)
  , lineno_(lineno)
  , basename_(file) {
  format_time();
  stream_ << ValueT(CurrentThread::get_strftid(), CurrentThread::get_strftid_length());
  stream_ << ValueT(kLogLevelNames[static_cast<int>(level_)], 6);
  if (0 != saved_errno)
    stream_ << strerror_tl(saved_errno) << " (errno=" << saved_errno << ") ";
}

void Logger::LoggerImpl::format_time(void) {
}

void Logger::LoggerImpl::finish(void) {
  stream_ << " - " << basename_ << ":" << lineno_ << "\n";
}

}
