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
#ifndef __TYR_DATE_HEADER_H__
#define __TYR_DATE_HEADER_H__

#include "TTypes.h"

struct tm;

namespace tyr {

class Date {
  int unix_day_;
public:
  struct InternalDate {
    int year;
    int month;
    int day;
  };
  static const int kDaysPerWeek = 7;
  static const int kUnixDay19700101;

  Date(void)
    : unix_day_(0) {
  }

  explicit Date(int unix_day)
    : unix_day_(unix_day) {
  }

  Date(int year, int month, int day);
  explicit Date(const struct tm& t);

  void swap(Date& r) {
    std::swap(unix_day_, r.unix_day_);
  }

  bool is_valid(void) const {
    return unix_day_ > 0;
  }

  int year(void) const {
    return internal_date().year;
  }

  int month(void) const {
    return internal_date().month;
  }

  int day(void) const {
    return internal_date().day;
  }

  int week_day(void) const {
    return (unix_day_ + 1) % kDaysPerWeek;
  }

  int unix_day(void) const {
    return unix_day_;
  }

  std::string to_iso_string(void) const;
  InternalDate internal_date(void) const;
};

inline bool operator==(const Date& a, const Date& b) {
  return a.unix_day() == b.unix_day();
}

inline bool operator<(const Date& a, const Date& b) {
  return a.unix_day() < b.unix_day();
}

}

#endif // __TYR_DATE_HEADER_H__
