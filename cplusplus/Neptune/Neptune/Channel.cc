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
#include <sstream>
#include <Chaos/Platform.h>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Channel.h>

namespace Neptune {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop)
  , fd_(fd) {
}

Channel::~Channel(void) {
  CHAOS_CHECK(!event_handling_, "`event_handling_` should be `false` while dtor");
  CHAOS_CHECK(!added_to_loop_, "`added_to_loop_` should be `false` while dtor");
  if (loop_->in_loopthread())
    CHAOS_CHECK(!loop_->has_channel(this), "current channel not in event loop");
}

void Channel::handle_event(Chaos::Timestamp recvtime) {
  std::shared_ptr<void> guard;
  if (tied_) {
    guard = wk_tie_.lock();
    if (guard)
      handle_event_with_guard(recvtime);
  }
  else {
    handle_event_with_guard(recvtime);
  }
}

void Channel::tie(const std::shared_ptr<void>& tiep) {
  wk_tie_ = tiep;
  tied_ = true;
}

void Channel::remove(void) {
  CHAOS_CHECK(is_none_event(), "channel event should be None");
  added_to_loop_ = false;
  loop_->remove_channel(this);
}

std::string Channel::events_to_string(void) const {
  return events_to_string(fd_, events_);
}

std::string Channel::revents_to_string(void) const {
  return events_to_string(fd_, revents_);
}

void Channel::update(void) {
  added_to_loop_ = true;
  loop_->update_channel(this);
}

void Channel::handle_event_with_guard(Chaos::Timestamp recvtime) {
  event_handling_ = true;
  CHAOSLOG_TRACE << "Channel::handle_event_with_guard - " << events_to_string();

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (log_hup_)
      CHAOSLOG_WARN << "Channel::handle_event_with_guard - fd=" << fd_ << " POLLHUP";
    if (close_fn_)
      close_fn_();
  }

  if (revents_ & POLLNVAL)
    CHAOSLOG_WARN << "Channel::handle_event_with_guard - fd=" << fd_ << " POLLNVAL";

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_fn_)
      error_fn_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLHUP)) {
    if (read_fn_)
      read_fn_(recvtime);
  }
  if (revents_ & POLLOUT) {
    if (write_fn_)
      write_fn_();
  }
  event_handling_ = false;
}

std::string Channel::events_to_string(int fd, int event) {
  std::ostringstream oss;

  oss << fd << " : ";
  if (event & POLLIN)
    oss << "IN ";
  if (event & POLLPRI)
    oss << "PRI ";
  if (event & POLLOUT)
    oss << "OUT ";
  if (event & POLLHUP)
    oss << "HUP ";
#if defined(CHAOS_LINUX)
  if (event & POLLRDHUP)
    oss << "RDHUP ";
#endif
  if (event & POLLERR)
    oss << "ERR ";
  if (event & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}

}
