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
#ifndef __TYR_LOGFILE_HEADER_H__
#define __TYR_LOGFILE_HEADER_H__

#include <memory>
#include "TTypes.h"
#include "TMutex.h"

namespace tyr {

class AppendFile;

class LogFile : private UnCopyable {
  static const int kRollPerSeconds = 60*60*24;
  const std::string basename_;
  const size_t rollsz_;
  const int flush_interval_;
  const int check_everyn_;

  int count_;
  std::unique_ptr<Mutex> mtx_;
  time_t start_of_period_;
  time_t last_roll_;
  time_t last_flush_;
  std::unique_ptr<AppendFile> file_;

  void append_unlocked(const char* buffer, int len);
  static std::string get_logfile_name(const std::string& basename, time_t* now);
public:
  LogFile(const std::string& basename, size_t roll_size,
      bool thread_safe = true, int flush_interval = 3, int check_every_n = 1024);
  ~LogFile(void);

  void append(const char* buffer, int len);
  void flush(void);
  bool roll_file(void);
};

}

#endif // __TYR_LOGFILE_HEADER_H__
