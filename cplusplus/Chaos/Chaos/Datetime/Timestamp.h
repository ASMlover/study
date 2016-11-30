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
#ifndef CHAOS_DATETIME_TIMESTAMP_H
#define CHAOS_DATETIME_TIMESTAMP_H

#include <stdint.h>
#include <string>
#include <utility>
#include <Chaos/Copyable.h>

namespace Chaos {

class Timestamp : public Copyable {
  int64_t epoch_msec_{};
public:
  Timestamp(void) = default;

  explicit Timestamp(int64_t epoch_msec)
    : epoch_msec_(epoch_msec) {
  }

  void swap(Timestamp& r) {
    std::swap(epoch_msec_, r.epoch_msec_);
  }

  bool is_valid(void) const {
    return epoch_msec_ > 0;
  }

  int64_t msec_since_epoch(void) const {
    return epoch_msec_;
  }

  time_t sec_since_epoch(void) const {
    return static_cast<time_t>(epoch_msec_ / kMicrosecondsPerSecond);
  }

  std::string to_string(void) const;
  std::string to_formatted_string(bool show_msec = true) const;

  static Timestamp now(void);
  static Timestamp invalid(void);
  static Timestamp from_unix_time(time_t t);
  static Timestamp from_unix_time(time_t t, int msec);
  static const int kMicrosecondsPerSecond = 1000 * 1000;
};

inline bool operator==(Timestamp a, Timestamp b) {
  return a.msec_since_epoch() == b.msec_since_epoch();
}

inline bool operator<(Timestamp a, Timestamp b) {
  return a.msec_since_epoch() < b.msec_since_epoch();
}

inline double time_difference(Timestamp a, Timestamp b) {
  int64_t diff = a.msec_since_epoch() - b.msec_since_epoch();
  return static_cast<double>(diff) / Timestamp::kMicrosecondsPerSecond;
}

inline Timestamp time_add(Timestamp t, double sec) {
  int64_t delta = static_cast<int64_t>(sec * Timestamp::kMicrosecondsPerSecond);
  return Timestamp(t.msec_since_epoch() + delta);
}

}

#endif // CHAOS_DATETIME_TIMESTAMP_H
