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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include "TTypes.h"
#include "TPlatform.h"
#include "TCurrentThread.h"
#include "TTimezone.h"
#include "TLogging.h"

namespace tyr { namespace basic {

#if defined(TYR_WINDOWS)
  __declspec(thread) char tErrnoBuf[512];
  __declspec(thread) char tTime[32];
  __declspec(thread) time_t tLastSecond;
#else
  __thread char tErrnoBuf[512];
  __thread char tTime[32];
  __thread time_t tLastSecond;
#endif

const char* strerror_tl(int saved_errno) {
#if defined(TYR_LINUX)
  return strerror_r(saved_errno, tErrnoBuf, sizeof(tErrnoBuf));
#else
  strerror_r(saved_errno, tErrnoBuf, sizeof(tErrnoBuf));
  return tErrnoBuf;
#endif
}

LoggingLevel init_log_level(void) {
  if (getenv("TYR_LOG_TRACE"))
    return LoggingLevel::LOGGINGLEVEL_TRACE;
  else if (getenv("TYR_LOG_DEBUG"))
    return LoggingLevel::LOGGINGLEVEL_DEBUG;
  else
    return LoggingLevel::LOGGINGLEVEL_INFO;
}

LoggingLevel gLogLevel = init_log_level();

const char* kLogLevelName[static_cast<int>(LoggingLevel::COUNT_LEVELS)] = {
  "TRACE",
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};

class T {
public:
  const char* str_;
  const uint32_t len_;

  T(const char* str, uint32_t len)
    : str_(str)
    , len_(len) {
    assert(strlen(str_) == len_);
  }
};

inline LogStream& operator<<(LogStream& s, T v) {
  s.append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
  s.append(v.data_, v.size_);
  return s;
}

void default_output(const char* msg, int len) {
  fwrite(msg, 1, len, stdout);
}

void default_flush(void) {
  fflush(stdout);
}

Logger::OutputCallback gOutput = default_output;
Logger::FlushCallback gFlush = default_flush;
Timezone gLogTimeZone;

class Logger::LoggerImpl {
public:
  Timestamp time_;
  LogStream stream_;
  LoggingLevel level_;
  int lineno_;
  SourceFile basename_;

  LoggerImpl(LoggingLevel level, int old_errno, const SourceFile& file, int lineno);
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
  stream_ << T(CurrentThread::tid_string(), CurrentThread::tid_string_length());
  stream_ << T(kLogLevelName[static_cast<int>(level)], 6);
  if (0 != saved_errno)
    stream_ << strerror_tl(saved_errno) << " (errno=" << saved_errno << ") ";
}

void Logger::LoggerImpl::format_time(void) {
  int64_t msec_since_epoch = time_.msec_since_epoch();
  time_t seconds = static_cast<time_t>(msec_since_epoch / Timestamp::kMicroSecondsPerSecond);
  int msec = static_cast<int>(msec_since_epoch % Timestamp::kMicroSecondsPerSecond);
  if (seconds != tLastSecond) {
    tLastSecond = seconds;
    struct tm tm_time;
    if (gLogTimeZone.is_valid())
      tm_time = gLogTimeZone.to_localtime(seconds);
    else
      gmtime_r(&seconds, &tm_time);

    int len = snprintf(tTime, sizeof(tTime), "%04d%02d%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(17 == len);
    UNUSED(len);
  }

  if (gLogTimeZone.is_valid()) {
    Format fmt(".%06.d ", msec);
    assert(fmt.size() == 8);
    stream_ << T(tTime, 17) << T(fmt.data(), fmt.size());
  }
  else {
    Format fmt(".%06dZ ", msec);
    assert(fmt.size() == 9);
    stream_ << T(tTime, 17) << T(fmt.data(), fmt.size());
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

Logger::Logger(SourceFile file, int lineno, bool to_abort)
  : impl_(new LoggerImpl(
        to_abort ? LoggingLevel::LOGGINGLEVEL_FATAL : LoggingLevel::LOGGINGLEVEL_ERROR,
        errno,
        file,
        lineno)) {
}

Logger::~Logger(void) {
  impl_->finish();
  const LogStream::Buffer& buf(stream().buffer());
  gOutput(buf.data(), buf.size());
  if (LoggingLevel::LOGGINGLEVEL_FATAL == impl_->level_) {
    gFlush();
    abort();
  }
}

LogStream& Logger::stream(void) {
  return impl_->stream_;
}

LoggingLevel Logger::log_level(void) {
  return gLogLevel;
}

void Logger::set_log_level(LoggingLevel level) {
  gLogLevel = level;
}

void Logger::set_output(OutputCallback cb) {
  gOutput = cb;
}

void Logger::set_flush(FlushCallback cb) {
  gFlush = cb;
}

void Logger::set_timezone(const Timezone& tz) {
  gLogTimeZone = tz;
}

}}
