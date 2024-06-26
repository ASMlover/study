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
#include <time.h>
#include <iostream>
#if !TYR_CODING
// Just for vim YCM plugin while editing
# include "../basic/TCircularBuffer.h"
# include "../basic/TStringPiece.h"
# include "../basic/TDate.h"
# include "../basic/TTimezone.h"
# include "../basic/TTimestamp.h"
# include "../basic/TCurrentThread.h"
# include "../basic/TBlockingQueue.h"
# include "../basic/TBoundedBlockingQueue.h"
# include "../basic/TMutex.h"
# include "../basic/TCondition.h"
# include "../basic/TThread.h"
# include "../basic/TThreadLocal.h"
#else
# include <basic/TCircularBuffer.h>
# include <basic/TStringPiece.h>
# include <basic/TDate.h>
# include <basic/TTimezone.h>
# include <basic/TTimestamp.h>
# include <basic/TCurrentThread.h>
# include <basic/TBlockingQueue.h>
# include <basic/TBoundedBlockingQueue.h>
# include <basic/TMutex.h>
# include <basic/TCondition.h>
# include <basic/TThread.h>
# include <basic/TThreadLocal.h>
#endif
using namespace tyr;

void tyr_sample_StringArg(void) {
  std::cout << "\n#################### StringArg ####################\n";
  std::string std_str("std::string('tyr test StringArg')");
  const char* c_str = "raw c-types string('tyr test StringArg')";

  tyr::basic::StringArg s1(c_str);
  std::cout << "StringArg with raw c-types string => " << s1.c_str() << std::endl;

  tyr::basic::StringArg s2(std_str);
  std::cout << "StringArg with std::string => " << s2.c_str() << std::endl;
  tyr::basic::StringArg s3(std_str.c_str());
  std::cout << "StringArg with std::string.c_str() => " << s3.c_str() << std::endl;
  tyr::basic::StringArg s4(std_str.data());
  std::cout << "StringArg with std::string.data() => " << s4.c_str() << std::endl;
}

static void tyr_show_StringPiece(const tyr::basic::StringPiece& s, const char* name = "StringPiece") {
  const char* output = "(null)";
  if (static_cast<bool>(s))
    output = s.data();
  std::cout << "object(`" << name << "`) tyr::basic::StringPiece is: @{\n\t\t"
    << "@data: " << output << "\n\t\t"
    << "@size: " << s.size() << "\n\t\t"
    << "@empty: " << s.empty() << "\n\t\t"
    << "@begin: " << (void*)s.begin() << "\n\t\t"
    << "@end: " << (void*)s.end() << "\n\t"
    << "}\n\t"
    << "@as_string: " << s.as_string() << std::endl;
}

void tyr_sample_StringPiece(void) {
  std::cout << "\n#################### StringPiece ####################\n";
  const char* cstr = "raw c-types string<tyr test StringPiece>";
  size_t cstr_len = strlen(cstr);
  const byte_t* bstr = reinterpret_cast<const byte_t*>("raw byte-types string<tyr test StringPiece>");
  std::string std_str("std::string<tyr test StringPiece>");
  char buf[] = "memory buffer<tyr test StringPiece>";
  size_t buf_len = sizeof(buf);

  tyr::basic::StringPiece s1;
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.set(cstr);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.set(cstr, cstr_len);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.set(buf, buf_len);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.remove_prefix(10);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.remove_suffix(5);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  std::cout << "@s1(" << s1 << ") starts_with<fer>: " << s1.starts_with("fer") << std::endl;
  s1.clear();
  tyr_show_StringPiece(s1, "StringPiece.s1");

  tyr::basic::StringPiece s2(bstr);
  tyr_show_StringPiece(s2, "StringPiece.s2");
  tyr::basic::StringPiece s3(std_str);
  tyr_show_StringPiece(s3, "StringPiece.s3");

  std::cout << "s2 compare with s3 -"
    << " @==: " << (s2 == s3)
    << " @<: " << (s2 < s3)
    << " @<=: " << (s2 <= s3)
    << " @>: " << (s2 > s3)
    << " @>=: " << (s2 >= s3) << std::endl;

  std::string s;
  s2.copy_to_string(&s);
  std::cout << "with copy_to_string @s: " << s << std::endl;
  s3.copy_to_string(&s);
  std::cout << "with copy_to_string @s: " << s << std::endl;
}

static void tyr_show_Timestamp(tyr::basic::Timestamp t, const char* name = "Timestamp") {
  std::cout << "object(`" << name << "`) tyr::basic::Timestamp is: @{\n\t\t"
    << "@is_valid: " << t.is_valid() << "\n\t\t"
    << "@msec_since_epoch: " << t.msec_since_epoch() << "\n\t\t"
    << "@sec_since_epoch: " << t.sec_since_epoch() << "\n\t\t"
    << "@to_string: " << t.to_string() << "\n\t\t"
    << "@to_formatted_string(true): " << t.to_formatted_string() << "\n\t"
    << "}" << std::endl;
}

void tyr_sample_Timestamp(void) {
  std::cout << "\n#################### Timestamp ####################\n";
  using namespace tyr::basic;

  Timestamp t1;
  tyr_show_Timestamp(t1, "Timestamp.t1");

  Timestamp t2(time(nullptr) * Timestamp::kMicroSecondsPerSecond);
  tyr_show_Timestamp(t2, "Timestamp.t2");

  t1.swap(t2);
  tyr_show_Timestamp(t1, "Timestamp.t1.after.swap");
  tyr_show_Timestamp(t2, "Timestamp.t2.after.swap");

  tyr_show_Timestamp(Timestamp::now(), "Timestamp.now");
  tyr_show_Timestamp(Timestamp::invalid(), "Timestamp.invalid");
  tyr_show_Timestamp(Timestamp::from_unix_time(time(nullptr)), "Timestamp.from_unix_time");
  tyr_show_Timestamp(Timestamp::from_unix_time(time(nullptr), 100), "Timestamp.from_unix_time.2");

  Timestamp t3(Timestamp::from_unix_time(time(nullptr)));
  Timestamp t4(Timestamp::from_unix_time(time(nullptr), 100));
  tyr_show_Timestamp(t3, "Timestamp.t3");
  tyr_show_Timestamp(t4, "Timestamp.t4");
  std::cout << "@t3 == @now: " << (t3 == Timestamp::now()) << std::endl;
  std::cout << "@t3 < @t4: " << (t3 < t4) << std::endl;
  std::cout << "time_difference(t4 - t3) = " << time_difference(t4, t3) << std::endl;
  tyr_show_Timestamp(add_time(t3, 5), "Timestamp.add_time.(now + 5s)");
}

void tyr_sample_CurrentThread(void) {
  std::cout << "\n#################### CurrentThread ####################\n";
  using namespace tyr::basic;

  CurrentThread::cached_tid();

  std::cout << "namespace CurrentThread functions: @{\n\t\t"
    << "@tid: " << CurrentThread::tid() << "\n\t\t"
    << "@tid_string: " << CurrentThread::tid_string() << "\n\t\t"
    << "@tid_string_length: " << CurrentThread::tid_string_length() << "\n\t\t"
    << "@name: " << CurrentThread::name() << "\n\t\t"
    << "@is_main_thread: " << CurrentThread::is_main_thread() << "\n\t"
    << "}" << std::endl;

  tyr_show_Timestamp(Timestamp::now(), "CurrentThread.begin");
  CurrentThread::sleep_usec(1010);
  tyr_show_Timestamp(Timestamp::now(), "CurrentThread.end");
}

static void tyr_show_Date(tyr::basic::Date d, const char* name = "Date") {
  std::cout << "object(`" << name << "`): @{\n\t\t"
    << "@is_valid: " << d.is_valid() << "\n\t\t"
    << "@year: " << d.year() << "\n\t\t"
    << "@month: " << d.month() << "\n\t\t"
    << "@day: " << d.day() << "\n\t\t"
    << "@weekday: " << d.weekday() << "\n\t\t"
    << "@epoch_day: " << d.epoch_day() << "\n\t\t"
    << "@iso_string: " << d.to_iso_string() << "\n\t"
    << "}" << std::endl;
}

void tyr_sample_Date(void) {
  std::cout << "\n#################### Date ####################\n";
  using namespace tyr::basic;

  Date d1;
  tyr_show_Date(d1, "Date.d1");
  Date d2(2457600);
  tyr_show_Date(d2, "Date.d2");
  Date d3(2016, 10, 14);
  tyr_show_Date(d3, "Date.d3");
  d2.swap(d3);
  tyr_show_Date(d2, "Date.d2.after.swap");
  tyr_show_Date(d3, "Date.d3.after.swap");

  time_t current_time = time(nullptr);
  struct tm* now = localtime(&current_time);
  Date d4(*now);
  tyr_show_Date(d4, "Date.d4");

  Date::DateTuple dt = d4.get_date();
  std::cout << "(`Date.d4`) @DateTuple {@year: "
    << dt.year << ", @month: " << dt.month << ", @day: " << dt.day << "}" << std::endl;
  std::cout << "@d3 == @d4: " << (d3 == d4) << ", @d3 < @d4: " << (d3 < d4) << std::endl;
}

template <typename T>
static void tyr_show_CircularBuffer(
    const tyr::basic::CircularBuffer<T>& cb, const char* name = "CircularBuffer<T>") {
  if (cb) {
    std::cout << "object(`" << name << "`) @{"
      << "\n\t\t@empty: " << cb.empty()
      << "\n\t\t@full: " << cb.full()
      << "\n\t\t@size: " << cb.size()
      << "\n\t\t@capacity: " << cb.capacity()
      << "\n\t\t@max_size: " << cb.max_size()
      << "\n\t\t@data: " << cb.data()
      << "\n\t\t@front: " << cb.front()
      << "\n\t\t@back: " << cb.back()
      << "\n\t\t@elements: {";
  }
  else {
    std::cout << "object(`" << name << "`) @{"
      << "\n\t\t@empty: " << cb.empty()
      << "\n\t\t@full: " << cb.full()
      << "\n\t\t@size: " << cb.size()
      << "\n\t\t@capacity: " << cb.capacity()
      << "\n\t\t@max_size: " << cb.max_size()
      << "\n\t\t@data: " << cb.data()
      << "\n\t\t@elements: {";
  }
  for (auto& e : cb)
    std::cout << e << ", ";
  std::cout << "}";
  std::cout << "\n\t}" << std::endl;
}

void tyr_sample_CircularBuffer(void) {
  std::cout << "\n#################### CircularBuffer ####################\n";
  using namespace tyr::basic;

  CircularBuffer<int> cb(5);
  tyr_show_CircularBuffer(cb, "CircularBuffer<int>.cb");
  CircularBuffer<int> cb1 = std::move(cb);
  tyr_show_CircularBuffer(cb, "CircularBuffer<int>.cb.after.move");
  tyr_show_CircularBuffer(cb1, "CircularBuffer<int>.cb1");

  cb1.push_back(1);
  cb1.push_back(2);
  cb1.push_back(3);

  CircularBuffer<int> cb2(cb1.begin(), cb1.end());
  tyr_show_CircularBuffer(cb2, "CircularBuffer<int>.cb2");

  cb2.clear();
  tyr_show_CircularBuffer(cb2, "CircularBuffer<int>.cb2.after.clear");

  for (int i = 0; i < 7; ++i) {
    int x = (i + 1) * (i + 1);
    std::cout << "CircularBuffer<int>.cb2.push_back operatation => @x: " << x << std::endl;
    cb2.push_back(x);
  }
  tyr_show_CircularBuffer(cb2, "CircularBuffer<int>.cb2.after.push_back");
}

template <typename T>
static void tyr_show_BlockingQueue(tyr::basic::BlockingQueue<T>& b, const char* name = "BlockingQueue<T>") {
  if (b.size() > 0) {
    std::cout << "object(`" << name << "`) @{"
      << "\n\t\t@size: " << b.size()
      << "\n\t\t@take: " << b.take();
    std::cout
      << "\n\t\t@size: " << b.size()
      << "\n\t}" << std::endl;
  }
  else {
    std::cout << "object(`" << name << "`) @{"
      << "\n\t\t@size: " << b.size()
      << "\n\t}" << std::endl;
  }
}

void tyr_sample_BlockingQueue(void) {
  std::cout << "\n#################### BlockingQueue ####################\n";
  using namespace tyr;

  basic::BlockingQueue<int> b1;
  tyr_show_BlockingQueue(b1, "BlockingQueue<int>.b1");

  b1.put(34);
  b1.put(45);
  b1.put(56);
  tyr_show_BlockingQueue(b1, "BlockingQueue<int>.b1.after.put");
}

template <typename T>
static void tyr_show_BoundedBlockingQueue(
    const tyr::basic::BoundedBlockingQueue<T>& b, const char* name = "BoundedBlockingQueue<T>") {
  std::cout << "object(`" << name << "`) @{"
    << "\n\t\t@empty: " << b.empty()
    << "\n\t\t@full: " << b.full()
    << "\n\t\t@size: " << b.size()
    << "\n\t\t@capacity: " << b.capacity()
    << "\n\t}" << std::endl;
}

void tyr_sample_BoundedBlockingQueue(void) {
  std::cout << "\n#################### BoundedBlockingQueue ####################\n";
  using namespace tyr;

  basic::BoundedBlockingQueue<int> b1(5);
  tyr_show_BoundedBlockingQueue(b1, "BoundedBlockingQueue<int>.b1");

  for (int i = 0; i < 5; ++i) {
    int x = ((i + 1) * (i + 1)) ^ 2;
    b1.put(x);
    std::cout << "BoundedBlockingQueue<int>.b1.put @value: " << x << std::endl;
  }
  tyr_show_BoundedBlockingQueue(b1, "BoundedBlockingQueue<int>.b1.after.put operations");

  while (!b1.empty())
    std::cout << "BoundedBlockingQueue<int>.b1.take @value: " << b1.take() << std::endl;
  tyr_show_BoundedBlockingQueue(b1, "BoundedBlockingQueue<int>.b1.after.take operations");
}

static void tyr_show_tm(const struct tm& t, const char* name = "Timezone.tm") {
  std::cout << "object(`" << name << "`) @{"
    << "\n\t\t@tm.tm_year: " << t.tm_year + 1900
    << "\n\t\t@tm.tm_mon: " << t.tm_mon + 1
    << "\n\t\t@tm.tm_mday: " << t.tm_mday
    << "\n\t\t@tm.tm_hour: " << t.tm_hour
    << "\n\t\t@tm.tm_min: " << t.tm_min
    << "\n\t\t@tm.tm_sec: " << t.tm_sec
    << "\n\t}" << std::endl;
}

void tyr_sample_Timezone(void) {
  std::cout << "\n#################### Timezone ####################\n";
  using namespace tyr;

  time_t now = time(nullptr);
  struct tm* t1 = localtime(&now);
  tyr_show_tm(*t1, "stdc-library.localtime.t1");

  struct tm t2 = basic::Timezone::to_utc_time(now, true);
  tyr_show_tm(t2, "Timezone.utc_time.t2");
  std::cout << "Timezone.from_utc_time.t2: " << basic::Timezone::from_utc_time(t2) << std::endl;
  std::cout << "Timezone.from_utc_time(2016-10-20 12:12:12): " << basic::Timezone::from_utc_time(2016, 10, 20, 12, 12, 12) << std::endl;

  basic::Timezone tz1(8 * 3600, "CST");
  struct tm t3 = tz1.to_localtime(time(nullptr));
  tyr_show_tm(t3, "Timezone.tz1.to_localtime.t3");
  std::cout << "object(`Timezone.tz1`) @is_valid: " << tz1.is_valid()
    << ", @localtime: " << tz1.from_localtime(t3) << std::endl;
}

static void tyr_show_Mutex(tyr::basic::Mutex& m, const char* name = "Mutex") {
  std::cout << "object(`" << name << "`) @{"
    << "\n\t\t@locked_by_this_thread: " << m.locked_by_this_thread()
    << "\n\t\t@get_mutex: " << m.get_mutex()
    << "\n\t}" << std::endl;
}

void tyr_sample_Mutex(void) {
  std::cout << "\n#################### Mutex ####################\n";
  using namespace tyr;

  basic::Mutex m;
  int counted1 = 1;

  m.lock();
  ++counted1;
  std::cout << "Mutex.m.raw.lock - @counted1: " << counted1 << std::endl;
  tyr_show_Mutex(m, "Mutex.raw.enter.lock");
  m.unlock();
  tyr_show_Mutex(m, "Mutex.raw.leave.lock");

  {
    basic::MutexGuard g1(m);
    ++counted1;
    std::cout << "Mutex.MutexGuard.m - @counted1: " << counted1 << std::endl;
    tyr_show_Mutex(m, "Mutex.MutexGuard.m");
  }
}

class ConditionWorker : private basic::UnCopyable {
  mutable basic::Mutex mtx_;
  basic::Condition cond_;
  bool broadcast_{false};
  volatile bool posted_{false};
public:
  ConditionWorker(void)
    : cond_(mtx_) {
  }

  void run(void) {
    // basic::CurrentThread::sleep_usec(1000);

    basic::MutexGuard guard(mtx_);
    if (broadcast_)
      cond_.broadcast();
    else
      cond_.signal();
    posted_ = true;
  }

  void run_signal(void) {
    basic::Thread t(std::bind(&ConditionWorker::run, this), "ConditionWorker.run_signal");
    t.start();

    {
      std::cout << "ConditionWorker.run_signal begin - @posted_: " << posted_ << std::endl;
      basic::MutexGuard guard(mtx_);
      while (!posted_)
        cond_.wait();
      std::cout << "ConditionWorker.run_signal end - @posted_: " << posted_ << std::endl;
    }

    t.join();
  }

  void run_broadcast(void) {
    broadcast_ = true;
    basic::Thread t(std::bind(&ConditionWorker::run, this), "ConditionWorker.run_broadcast");
    t.start();

    {
      std::cout << "ConditionWorker.run_broadcast begin - @posted_: " << posted_ << std::endl;
      basic::MutexGuard guard(mtx_);
      while (!posted_)
        cond_.wait();
      std::cout << "ConditionWorker.run_broadcast end - @posted_: " << posted_ << std::endl;
    }

    t.join();
  }
};

void tyr_sample_Condition(void) {
  std::cout << "\n#################### Condition ####################\n";
  {
    ConditionWorker cw;
    cw.run_signal();
  }

  {
    ConditionWorker cw;
    cw.run_broadcast();
  }
}

static void tyr_show_CurrentThread(void) {
  std::cout << "current information: {"
    << "\n\t\t@tid: " << basic::CurrentThread::tid()
    << "\n\t\t@tid_string: " << basic::CurrentThread::tid_string()
    << "\n\t\t@tid_string_length: " << basic::CurrentThread::tid_string_length()
    << "\n\t\t@name: " << basic::CurrentThread::name()
    << "\n\t\t@is_main_thread: " << basic::CurrentThread::is_main_thread()
    << "\n\t}" << std::endl;
}

static void tyr_sample_thread_closure(void) {
  std::cout << "********** tyr_sample_thread_closure **********\n";
  tyr_show_CurrentThread();
}

void tyr_sample_Thread(void) {
  std::cout << "\n#################### Thread ####################\n";

  basic::Thread t(tyr_sample_thread_closure, "Thread.t");
  t.start();
  std::cout << "Thread.t information - "
    << "@started: " << t.started()
    << "@tid: " << t.tid()
    << "@name: " << t.name()
    << "@num_created: " << t.num_created()
    << std::endl;
  t.join();
}

void tyr_run_all_samples(void) {
  tyr_sample_StringArg();
  tyr_sample_StringPiece();
  tyr_sample_Date();
  tyr_sample_Timezone();
  tyr_sample_Timestamp();
  tyr_sample_CurrentThread();
  tyr_sample_CircularBuffer();
  tyr_sample_BlockingQueue();
  tyr_sample_BoundedBlockingQueue();
  tyr_sample_Mutex();
  tyr_sample_Condition();
  tyr_sample_Thread();
}
