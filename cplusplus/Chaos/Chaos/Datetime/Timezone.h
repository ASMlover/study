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
#ifndef CHAOS_DATETIME_TIMEZONE_H
#define CHAOS_DATETIME_TIMEZONE_H

#include <time.h>
#include <memory>
#include <Chaos/Copyable.h>

namespace Chaos {

struct TZData;

class Timezone : public Copyable {
  std::shared_ptr<TZData> data_;
public:
  Timezone(void) = default;
  explicit Timezone(const char* zonefile);
  Timezone(int east_of_utc, const char* tzname);

  bool is_valid(void) const;
  struct tm to_localtime(time_t sec_since_epoch) const;
  time_t from_localtime(const struct tm& t) const;

  static struct tm to_utc_time(time_t sec_since_epoch, bool yday = false);
  static time_t from_utc_time(const struct tm& utc);
  static time_t from_utc_time(int year, int month, int day, int hour, int min, int sec);
};

}

#endif // CHAOS_DATETIME_TIMEZONE_H
