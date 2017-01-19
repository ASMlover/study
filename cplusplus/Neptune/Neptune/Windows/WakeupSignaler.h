// Copyright (c) 2017 ASMlover. All rights reserved.
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
#ifndef NEPTUNE_WINDOWS_WAKEUPSIGNALER_H
#define NEPTUNE_WINDOWS_WAKEUPSIGNALER_H

#include <cstddef>
#include <Chaos/UnCopyable.h>

namespace Neptune {

class WakeupSignaler : private Chaos::UnCopyable {
  int readfd_{}; // reader socket filedes
  int writfd_{}; // writer socket filedes

  void open_signaler(void);
  void close_signaler(void);
public:
  WakeupSignaler(void);
  ~WakeupSignaler(void);

  int set_signal(const void* buf, std::size_t len);
  int get_signal(std::size_t len, void* buf);

  bool is_opened(void) const {
    return readfd_ > 0 && writfd_ > 0;
  }

  int get_reader(void) const {
    return readfd_;
  }

  int get_writer(void) const {
    return writfd_;
  }
};

}

#endif // NEPTUNE_WINDOWS_WAKEUPSIGNALER_H
