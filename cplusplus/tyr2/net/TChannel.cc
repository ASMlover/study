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
#include <sstream>
#include "../basic/TLogging.h"
#include "TSocketSupport.h"
#include "TEventLoop.h"
#include "TChannel.h"

namespace tyr { namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop)
  , fd_(fd) {
}

Channel::~Channel(void) {
  assert(!event_handling_);
  assert(!added_to_loop_);
  if (loop_->in_loopthread())
    assert(!loop_->has_channel(this));
}

void Channel::handle_event(basic::Timestamp recv_time) {
  std::shared_ptr<void> guard;
  if (tied_) {
    guard = wk_tie_.lock();
    if (guard)
      handle_event_with_guard(recv_time);
  }
  else {
    handle_event_with_guard(recv_time);
  }
}

void Channel::tie(const std::shared_ptr<void>& tiep) {
  wk_tie_ = tiep;
  tied_ = true;
}

void Channel::remove(void) {
  assert(is_none_event());
  added_to_loop_ = false;
  loop_->remove_channel(this);
}

std::string Channel::events_to_string(void) const {
  // TODO:
  return "";
}

std::string Channel::revents_to_string(void) const {
  // TODO:
  return "";
}

void Channel::update(void) {
  added_to_loop_ = true;
  loop_->update_channel(this);
}

void Channel::handle_event_with_guard(basic::Timestamp recv_time) {
  event_handling_ = true;
  TYRLOG_TRACE << "Channel::handle_event_with_guard - " << revents_to_string();

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (log_hup_)
      TYRLOG_WARN << "Channel::handle_event_with_guard - fd=" << fd_ << " POLLHUP";
    if (close_fn_)
      close_fn_();
  }

  if (revents_ & POLLNVAL)
    TYRLOG_WARN << "Channel::handle_event_with_guard - fd=" << fd_ << " POLLNVAL";

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_fn_)
      error_fn_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLHUP)) {
    if (read_fn_)
      read_fn_(recv_time);
  }
  if (revents_ & POLLOUT) {
    if (write_fn_)
      write_fn_();
  }
  event_handling_ = false;
}

std::string Channel::events_to_string(int fd, int event) {
  std::ostringstream oss;

  oss << fd << ": ";
  if (event & POLLIN)
    oss << "IN ";
  if (event & POLLPRI)
    oss << "PRI ";
  if (event & POLLOUT)
    oss << "OUT ";
  if (event & POLLHUP)
    oss << "HUP ";
#if !defined(TYR_WINDOWS)
  if (event & POLLRDHUP)
    oss << "RDHUP ";
#endif
  if (event & POLLERR)
    oss << "ERR ";
  if (event & POLLNVAL)
    oss << "NVAL ";
  return oss.str();
}

}}
