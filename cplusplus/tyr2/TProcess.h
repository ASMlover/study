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
#ifndef __TYR_PROCESS_HEADER_H__
#define __TYR_PROCESS_HEADER_H__

#include <vector>
#include "TTypes.h"
#include "TStringPiece.h"
#include "TTimestamp.h"

namespace tyr {
namespace Process {

pid_t pid(void);
std::string pid_string(void);
uid_t uid(void);
std::string username(void);
uid_t euid(void);
Timestamp start_time(void);
int clock_ticks_per_second(void);
int page_size(void);
bool is_debug_build(void);
std::string hostname(void);
std::string procname(void);
StringPiece procname(const std::string& stat);
std::string proc_status(void);
std::string proc_stat(void);
std::string thread_stat(void);
std::string exe_path(void);
int opened_files(void);
int max_open_files(void);

struct CPUTime {
  double usr_seconds;
  double sys_seconds;

  CPUTime(void)
    : usr_seconds(0.0)
    , sys_seconds(0.0) {
  }
};
CPUTime cpu_time(void);

int num_threads(void);
std::vector<pid_t> threads(void);

}
}

#endif // __TYR_PROCESS_HEADER_H__
