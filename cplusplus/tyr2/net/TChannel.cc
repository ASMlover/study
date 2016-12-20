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
#include "../basic/TLogging.h"
#include "TSocketSupport.h"
#include "TEventLoop.h"
#include "TChannel.h"

namespace tyr { namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLOUT;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop)
  , fd_(fd) {
}

Channel::~Channel(void) {
  assert(!event_handling_);
}

void Channel::handle_event(basic::Timestamp recv_time) {
  event_handling_ = true;
  if (revents_ & POLLNVAL)
    TYRLOG_WARN << "Channel::handle_event() POLLNVAL";

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    TYRLOG_WARN << "Channel::handle_event() POLLHUP";
    if (!close_fn_)
      close_fn_();
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (!error_fn_)
      error_fn_();
  }

  if (revents_ & (POLLIN | POLLPRI | POLLHUP)) {
    if (!read_fn_)
      read_fn_(recv_time);
  }

  if (revents_ & POLLOUT) {
    if (!write_fn_)
      write_fn_();
  }
}

void Channel::update(void) {
  loop_->update_channel(this);
}

}}
