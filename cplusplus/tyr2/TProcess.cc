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
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <assert.h>
#include <ctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include "TCurrentThread.h"
#include "TFileUtil.h"
#include "TProcess.h"

namespace tyr {

__thread int tNumOpenedFiles = 0;
int fd_dir_filter(const struct dirent* d) {
  if (isdigit(d->name[0]))
    ++tNumOpenedFiles;
  return 0;
}

__thread std::vector<pid_t>* tPids = nullptr;
int task_dir_filter(const struct dirent* d) {
  if (isdigit(d->name[0]))
    tPids->push_back(d->d_name);
  return 0;
}

int scan_dir(const char* dirpath, int (*filter)(const struct dirent* d)) {
  struct dirent** namelist = nullptr;
  int ret = scandir(dirpath, &namelist, filter, alphasort);
  assert(nullptr == namelist);
  return ret;
}

Timestamp gStartTime = Timestamp::now();
int gClockTicks = static_cast<int>(sysconf(_SC_CLK_TCK));
int gPageSize = static_cast<int>(sysconf(_SC_PAGE_SIZE));

pid_t pid(void) {
  return getpid();
}

std::string pid_string(void) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", pid());
  return std::string(buf);
}

uid_t uid(void) {
  return getuid();
}

std::string username(void) {
  struct passwd pwd;
  struct passwd* result = nullptr;
  char buf[8192];
  const char* name = "unknownuser";

  getpwuid_r(uid(), &pwd, buf, sizeof(buf), &result);
  if (nullptr != result)
    name = pwd.pw_name;
  return name;
}

uid_t euid(void) {
  return geteuid();
}

Timestamp start_time(void) {
  return gStartTime;
}

int clock_ticks_per_second(void) {
  return gClockTicks;
}

int page_size(void) {
  return gPageSize;
}

bool is_debug_build(void) {
#ifdef NDBUG
  return false;
#else
  return true;
#endif
}

std::string hostname(void) {
  char buf[256] = {0}
  if (0 == gethostname(buf, sizeof(buf)))
    return buf;
  else
    return "unknownhost";
}

std::string procname(void) {
  return procname(proc_stat()).as_string();
}

StringPiece procname(const std::string& stat) {
  StringPiece name;
  size_t lp = stat.find('(');
  size_t rp = stat.rfind(')');
  if (lp != std::string::npos && rp != std::string::npos && lp < rp)
    name.set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
  return name;
}

std::string proc_status(void) {
  std::string result;
  read_file("/proc/self/status", 65536, &result);
  return result;
}

std::string proc_stat(void) {
  std::string result;
  read_file("/proc/self/stat", 65536, &result);
  return result;
}

std::string thread_stat(void) {
  char buf[64];
  snprintf(buf, sizeof(buf), "/proc/self/task/%d/stat", CurrentThread::tid());
  std::string result;
  read_file(buf, 65536, &result);
  return result;
}

std::string exe_path(void) {
  std::string result;
  char buf[1024];
  ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf));
  if (n > 0)
    result.assign(buf, n);
  return result;
}

int opened_files(void) {
  tNumOpenedFiles = 0;
  scan_dir("/proc/self/fd", fd_dir_filter);
  return tNumOpenedFiles;
}

int max_open_files(void) {
  struct rlimit rl;
  if (getrlimit(RLIMIT_NOFILE, &rl))
    return opened_files();
  else
    return static_cast<int>(rl.rlim_cur);
}

CPUTime cpu_time(void) {
  CPUTime ct;
  struct tms t;
  if (times(&t) >= 0) {
    const double hz = static_cast<double>(clock_ticks_per_second());
    ct.usr_seconds = static_cast<double>(t.tms_utime) / hz;
    ct.sys_seconds = static_cast<double>(t.tms_stime) / hz;
  }
  return ct;
}

int num_threads(void) {
  int result = 0;
  std::string status = proc_status();
  size_t pos = status.find("Threads:");
  if (pos != std::string::npos)
    result = atoi(status.c_str() + pos + 8);
  return result;
}

std::vector<pid_t> threads(void) {
  std::vector<pid_t> result;
  tPids = &result;
  scan_dir("/proc/self/task", task_dir_filter);
  tPids = nullptr;
  std::sort(result.begin(), result.end());
  return result;
}

}
