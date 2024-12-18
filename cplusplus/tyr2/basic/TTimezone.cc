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
#include <string.h>
#include <algorithm>
#include <vector>
#include "TTypes.h"
#include "TPlatform.h"
#include "TDate.h"
#include "TTimezone.h"

namespace tyr { namespace basic {

struct Transition {
  time_t gmttime;
  time_t localtime;
  int localtime_index;

  Transition(time_t gmt, time_t lt, int index)
    : gmttime(gmt)
    , localtime(lt)
    , localtime_index(index) {
  }
};

struct Compare {
  bool compare_gmt;

  Compare(bool gmt)
    : compare_gmt(gmt) {
  }

  bool operator()(const Transition& a, const Transition& b) const {
    if (compare_gmt)
      return a.gmttime < b.gmttime;
    else
      return a.localtime < b.localtime;
  }

  bool equal(const Transition& a, const Transition& b) const {
    if (compare_gmt)
      return a.gmttime == b.gmttime;
    else
      return a.localtime == b.localtime;
  }
};

struct Localtime {
  time_t gmtoff;
  bool isdst;
  int arrb_index;

  Localtime(time_t _gmtoff, bool _isdst, int _arrb)
    : gmtoff(_gmtoff)
    , isdst(_isdst)
    , arrb_index(_arrb) {
  }
};

const int kSecondsPerDay = 24 * 60 * 60;

struct Data {
  std::vector<Transition> transitions;
  std::vector<Localtime> localtimes;
  std::vector<std::string> names;
  std::string abbreviation;
};

inline void fill_time(unsigned int sec, struct tm* utc) {
  unsigned int min = sec / 60;
  utc->tm_hour = min / 60;
  utc->tm_min = min % 60;
  utc->tm_sec = sec % 60;
}

class File : private UnCopyable {
  FILE* stream_;
public:
  File(const char* fname)
    : stream_(fopen(fname, "rb")) {
  }

  ~File(void) {
    if (stream_)
      fclose(stream_);
  }

  bool is_valid(void) {
    return nullptr != stream_;
  }

  std::string read_bytes(size_t bytes) {
    TYR_DECLARRAY(char, buf, bytes);
    size_t n = fread(buf, 1, bytes, stream_);
    if (n != bytes)
      throw std::logic_error("no enough data");
    return buf;
  }

  int32_t read_int32(void) {
    int32_t x = 0;
    size_t n = fread(&x, 1, sizeof(int32_t), stream_);
    if (n != sizeof(int32_t))
      throw std::logic_error("bad int32_t data");
    return x;
  }

  uint8_t read_uint8(void) {
    uint8_t x = 0;
    size_t n = fread(&x, 1, sizeof(uint8_t), stream_);
    if (n != sizeof(uint8_t))
      throw std::logic_error("bad uint8_t data");
    return x;
  }
};

bool read_timezone_file(const char* zonefile, struct Data* data) {
  File f(zonefile);
  if (f.is_valid()) {
    try {
      std::string head = f.read_bytes(4);
      if (head != "TZif")
        throw std::logic_error("bad head");
      std::string version = f.read_bytes(1);
      f.read_bytes(15);

      int32_t isgmtcnt = f.read_int32();
      int32_t isstdcnt = f.read_int32();
      int32_t leapcnt = f.read_int32();
      int32_t timecnt = f.read_int32();
      int32_t typecnt = f.read_int32();
      int32_t charcnt = f.read_int32();

      std::vector<int32_t> trans;
      std::vector<int> localtimes;
      trans.reserve(timecnt);
      for (int i = 0; i < timecnt; ++i)
        trans.push_back(f.read_int32());
      for (int i = 0; i < timecnt; ++i)
        localtimes.push_back(f.read_uint8());
      for (int i = 0; i < typecnt; ++i) {
        int32_t gmtoff = f.read_int32();
        bool isdst = 0 != f.read_uint8();
        uint8_t abbrind = f.read_uint8();

        data->localtimes.push_back(Localtime(gmtoff, isdst, abbrind));
      }
      for (int i = 0; i < timecnt; ++i) {
        int local_index = localtimes[i];
        time_t lt = trans[i] + data->localtimes[local_index].gmtoff;
        data->transitions.push_back(Transition(trans[i], lt, local_index));
      }
      data->abbreviation = f.read_bytes(charcnt);

      for (int i = 0; i < leapcnt; ++i) {
      }
      (void)isstdcnt;
      (void)isgmtcnt;
    }
    catch (std::logic_error& e) {
      fprintf(stderr, "%s\n", e.what());
    }
  }
  return true;
}

const Localtime* find_localtime(const Data& data, Transition sentry, Compare cmp) {
  const Localtime* local = nullptr;

  if (data.transitions.empty() || cmp(sentry, data.transitions.front())) {
    local = &data.localtimes.front();
  }
  else {
    auto trans_iter = std::lower_bound(data.transitions.begin(), data.transitions.end(), sentry, cmp);
    if (trans_iter != data.transitions.end()) {
      if (!cmp.equal(sentry, *trans_iter)) {
        assert(trans_iter != data.transitions.begin());
        --trans_iter;
      }
      local = &data.localtimes[trans_iter->localtime_index];
    }
    else {
      local = &data.localtimes[data.transitions.back().localtime_index];
    }
  }
  return local;
}

Timezone::Timezone(const char* zonefile)
  : data_(new Data()) {
  if (!read_timezone_file(zonefile, data_.get())) {
    data_.reset();
  }
}

Timezone::Timezone(int east_of_utc, const char* tzname)
  : data_(new Data()) {
  data_->localtimes.push_back(Localtime(east_of_utc, false, 0));
  data_->abbreviation = tzname;
}

bool Timezone::is_valid(void) const {
  return static_cast<bool>(data_);
}

struct tm Timezone::to_localtime(time_t sec_since_epoch) const {
  struct tm ltime;
  memset(&ltime, 0, sizeof(ltime));
  assert(data_ != nullptr);
  const Data& data(*data_);

  Transition sentry(sec_since_epoch, 0, 0);
  const Localtime* local = find_localtime(data, sentry, Compare(true));

  if (local) {
    time_t local_seconds = sec_since_epoch + local->gmtoff;
    gmtime_r(&local_seconds, &ltime);
    ltime.tm_isdst = local->isdst;
#if !defined(TYR_WINDOWS)
    ltime.tm_gmtoff = local->gmtoff;
    ltime.tm_zone = (char*)&data.abbreviation[local->arrb_index];
#endif
  }
  return ltime;
}

time_t Timezone::from_localtime(const struct tm& t) const {
  assert(nullptr != data_);
  const Data& data(*data_);

  struct tm tmp = t;
  time_t seconds = timegm(&tmp);
  Transition sentry(0, seconds, 0);
  const Localtime* local = find_localtime(data, sentry, Compare(false));
  if (t.tm_isdst) {
    struct tm try_tm = to_localtime(seconds - local->gmtoff);
    if (!try_tm.tm_isdst && try_tm.tm_hour == t.tm_hour && try_tm.tm_min == t.tm_min)
      seconds -= 3600;
  }
  return seconds - local->gmtoff;
}

struct tm Timezone::to_utc_time(time_t sec_since_epoch, bool yday) {
  struct tm utc;
  memset(&utc, 0, sizeof(utc));
#if !defined(TYR_WINDOWS)
  utc.tm_zone = const_cast<char*>("GMT");
#endif
  int seconds = static_cast<int>(sec_since_epoch % kSecondsPerDay);
  int days = static_cast<int>(sec_since_epoch / kSecondsPerDay);
  if (seconds < 0) {
    seconds += kSecondsPerDay;
    --days;
  }
  fill_time(seconds, &utc);
  Date date(days + Date::kEpochDay19700101);
  Date::DateTuple dt = date.get_date();
  utc.tm_year = dt.year - 1900;
  utc.tm_mon = dt.month - 1;
  utc.tm_mday = dt.day;
  utc.tm_wday = date.weekday();

  if (yday) {
    Date start(dt.year, 1, 1);
    utc.tm_yday = date.epoch_day() - start.epoch_day();
  }
  return utc;
}

time_t Timezone::from_utc_time(const struct tm& utc) {
  return from_utc_time(
      utc.tm_year+ 1900, utc.tm_mon + 1, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec);
}

time_t Timezone::from_utc_time(int year, int month, int day, int hour, int min, int sec) {
  Date date(year, month, day);
  int sec_in_day = hour * 3600 + min * 60 + sec;
  time_t days = date.epoch_day() - Date::kEpochDay19700101;
  return days * kSecondsPerDay + sec_in_day;
}

}}
