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
#include <sys/eventfd.h>
#include <unistd.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Linux/WakeupSignaler.h>

namespace Neptune {

WakeupSignaler::WakeupSignaler(void) {
  open_signaler();
  if (!is_opened())
    CHAOSLOG_SYSFATAL << "WakeupSignaler::WakeupSignaler - open signaler failed";
}

WakeupSignaler::~WakeupSignaler(void) {
  close_signaler();
}

int WakeupSignaler::set_signal(const void* buf, std::size_t len) {
  return write(eventfd_, buf, len);
}

int WakeupSignaler::get_signal(std::size_t len, void* buf) {
  return read(eventfd_, buf, len);
}

void WakeupSignaler::open_signaler(void) {
  eventfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (eventfd_ < 0)
    CHAOSLOG_SYSFATAL << "WakeupSignaler::open_signaler - create eventfd failed, eventfd_=" << eventfd_;
}

void WakeupSignaler::close_signaler(void) {
  if (eventfd_ > 0) {
    close(eventfd_);
    eventfd_ = 0;
  }
}

}
