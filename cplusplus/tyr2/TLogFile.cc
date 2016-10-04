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
#include <stdio.h>
#include <time.h>
#include "TFileUtil.h"
#include "TProcess.h"
#include "TLogFile.h"

namespace tyr {

void LogFile::append_unlocked(const char* buffer, int len) {
  file_->append(buffer, len);

  if (file_->written_bytes() > rollsz_) {
    roll_file();
  }
  else {
    ++count_;
    if (count_ >= check_everyn_) {
      count_ = 0;
      time_t now = time(nullptr);
      time_t this_period = now / kRollPerSeconds * kRollPerSeconds;
      if (this_period != start_of_period_) {
        roll_file();
      }
      else if (now - last_flush_ > flush_interval) {
        last_flush_ = now;
        file_->flush();
      }
    }
  }
}

std::string LogFile::get_logfile_name(const std::string& basename, time_t* now) {
  std::string fname;
  fname.reserve(basename.size() + 64);
  fname = basename;

  char timebuf[32];
  struct tm tm_time;
  *now = time(nullptr);
  gmtime_r(now, &tm_time);
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm_time);
  fname += timebuf;

  fname += Process::hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof(pidbuf), ".%d", Process::pid());
  fname += pidbuf;
  fname += ".log";

  return fname;
}

LogFile::LogFile(const std::string& basename,
    size_t rollsz, bool thread_safe, int flush_interval, int check_every_n)
  : basename_(basename)
  , rollsz_(rollsz)
  , flush_interval_(flush_interval)
  , check_everyn_(check_every_n)
  , count_(0)
  , mtx_(thread_safe ? new Mutex() : nullptr)
  , start_of_period_(0)
  , last_roll_(0)
  , last_flush_(0) {
  assert(basename.find('/') == std::string::npos);
  roll_file();
}

LogFile::~LogFile(void) {
}

void LogFile::append(const char* buffer, int len) {
  if (mtx_) {
    MutexGuard guard(*mtx_);
    append_unlocked(buffer, len);
  }
  else {
    append_unlocked(buffer, len);
  }
}

void LogFile::flush(void) {
  if (mtx_) {
    MutexGuard guard(*mtx_);
    file_->flush();
  }
  else {
    file_->flush();
  }
}

bool LogFile::roll_file(void) {
  time_t now = 0;
  std::string fname = get_logfile_name(basename_, &now);
  time_t start = now / kRollPerSeconds * kRollPerSeconds;

  if (now > last_roll_) {
    last_roll_ = now;
    last_flush_ = now;
    start_of_period_ = start;
    file_.reset(new AppendFile(fname));
    return true;
  }
  return false;
}

}
