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
Timezone g_logging_tzone;

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
  int64_t msec_since_epoch = time_.msec_since_epoch();
  time_t seconds = static_cast<time_t>(msec_since_epoch / Timestamp::kMicrosecondsPerSecond);
  int msec = static_cast<int>(msec_since_epoch % Timestamp::kMicrosecondsPerSecond);
  if (seconds != t_last_seconds) {
    t_last_seconds = seconds;
    struct tm t;
    if (g_logging_tzone.is_valid())
      t = g_logging_tzone.to_localtime(seconds);
    else
      Chaos::kern_gmtime(&seconds, &t);

    int n = snprintf(t_timebuf,
        sizeof(t_timebuf),
        "%04d%02d%02d %02d:%02d:%02d",
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec);
    CHAOS_CHECK(n == 17, "Logger::LoggerImpl::format_time: `t_timebuf` length error");
  }

  if (g_logging_tzone.is_valid()) {
    Format fmt(".%06d", msec);
    CHAOS_CHECK(fmt.size() == 8, "Logger::LoggerImpl::format_time: format size should be `8`");
    stream_ << ValueT(t_timebuf, 17) << ValueT(fmt.data(), fmt.size());
  }
  else {
    Format fmt(".%06dZ", msec);
    CHAOS_CHECK(fmt.size() == 9, "Logger::LoggerImpl::format_time: format size should be `9`");
    stream_ << ValueT(t_timebuf, 17) << ValueT(fmt.data(), fmt.size());
  }
}

void Logger::LoggerImpl::finish(void) {
  stream_ << " - " << basename_ << ":" << lineno_ << "\n";
}

Logger::Logger(SourceFile file, int lineno)
  : impl_(new LoggerImpl(LoggingLevel::LOGGINGLEVEL_INFO, 0, file, lineno)) {
}

Logger::Logger(SourceFile file, int lineno, LoggingLevel level)
  : impl_(new LoggerImpl(level, 0, file, lineno)) {
}

Logger::Logger(SourceFile file, int lineno, LoggingLevel level, const char* func)
  : impl_(new LoggerImpl(level, 0, file, lineno)) {
  impl_->stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int lineno, bool do_abort)
  : impl_(new LoggerImpl(
        do_abort ? LoggingLevel::LOGGINGLEVEL_FATAL : LoggingLevel::LOGGINGLEVEL_ERROR,
        errno,
        file,
        lineno)) {
}

Logger::~Logger(void) {
  impl_->finish();
  const LogStream::buffer_type& buf(get_stream().get_buffer());
  g_output_fn(buf.data(), buf.size());
  if (LoggingLevel::LOGGINGLEVEL_FATAL == impl_->level_) {
    g_flush_fn();
    abort();
  }
}

LogStream& Logger::get_stream(void) {
  return impl_->stream_;
}

LoggingLevel Logger::get_loglevel(void) {
  return g_loglevel;
}

void Logger::set_loglevel(LoggingLevel level) {
  g_loglevel = level;
}

void Logger::set_output_callback(OutputCallback fn) {
  g_output_fn = fn;
}

void Logger::set_flush_callback(FlushCallback fn) {
  g_flush_fn = fn;
}

void Logger::set_timezone(const Timezone& tz) {
  g_logging_tzone = tz;
}

}
