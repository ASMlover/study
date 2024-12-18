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
#ifndef __TYR_NET_DARWIN_WAKEUPSIGNALERDARWIN_HEADER_H__
#define __TYR_NET_DARWIN_WAKEUPSIGNALERDARWIN_HEADER_H__

#include <stddef.h>
#include "../../basic/TUnCopyable.h"

namespace tyr { namespace net {

class WakeupSignaler : private basic::UnCopyable {
  int fds_[2]{}; // [0] - reader, [1] - writer

  void open_signaler(void);
  void close_signaler(void);
  void set_nonblock(int fd);
public:
  WakeupSignaler(void);
  ~WakeupSignaler(void);

  int set_signal(const void* buf, size_t len);
  int get_signal(void* buf, size_t len);

  bool is_opened(void) const {
    return fds_[0] > 0 && fds_[1] > 0;
  }

  int get_reader_fd(void) const {
    return fds_[0];
  }

  int get_writer_fd(void) const {
    return fds_[1];
  }
};

}}

#endif // __TYR_NET_DARWIN_WAKEUPSIGNALERDARWIN_HEADER_H__
