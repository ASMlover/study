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
#include <inttypes.h>
#include <time.h>
#include <Chaos/OS/OS.h>
#include <Chaos/Datetime/Timestamp.h>

namespace Chaos {

static_assert(sizeof(Timestamp) == sizeof(int64_t), "Timestamp should be same size as `int64_t`");

std::string Timestamp::to_string(void) const {
  char buf[32];
  int64_t sec = epoch_msec_ / kMicrosecondsPerSecond;
  int64_t msec = epoch_msec_ % kMicrosecondsPerSecond;
  snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", sec, msec);
  return buf;
}

std::string Timestamp::to_formatted_string(bool show_msec) const {
  char buf[32];
  time_t time = static_cast<time_t>(epoch_msec_ / kMicrosecondsPerSecond);
  struct tm result;
  Chaos::kern_gmtime(&time, &result);

  if (show_msec) {
    int msec = static_cast<int>(epoch_msec_ % kMicrosecondsPerSecond);
    snprintf(buf,
        sizeof(buf),
        "%04d%02d%02d %02d:%02d:%02d.%06d",
        result.tm_year + 1900,
        result.tm_mon + 1,
        result.tm_mday,
        result.tm_hour,
        result.tm_min,
        result.tm_sec,
        msec);
  }
  else {
    snprintf(buf,
        sizeof(buf),
        "%04d%02d%02d %02d:%02d:%02d",
        result.tm_year + 1900,
        result.tm_mon + 1,
        result.tm_mday,
        result.tm_hour,
        result.tm_min,
        result.tm_sec);
  }

  return buf;
}

Timestamp Timestamp::now(void) {
  struct timeval tv;
  Chaos::kern_gettimeofday(&tv, nullptr);
  int64_t sec = tv.tv_sec;
  return Timestamp(sec * kMicrosecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid(void) {
  return Timestamp();
}

Timestamp Timestamp::from_unix_time(time_t t) {
  return from_unix_time(t, 0);
}

Timestamp Timestamp::from_unix_time(time_t t, int msec) {
  return Timestamp(static_cast<int64_t>(t) * kMicrosecondsPerSecond + msec);
}

}
