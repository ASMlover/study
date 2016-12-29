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
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include "../../basic/TLogging.h"
#include "../TChannel.h"
#include "TAsyncPollerLinux.h"

namespace tyr { namespace net {

static_assert(EPOLLIN == POLLIN, "`epoll` uses same flag values as `poll`");
static_assert(EPOLLPRI == POLLPRI, "`epoll` uses same flag values as `poll`");
static_assert(EPOLLOUT == POLLOUT, "`epoll` uses same flag values as `poll`");
static_assert(EPOLLRDHUP == POLLRDHUP, "`epoll` uses same flag values as `poll`");
static_assert(EPOLLERR == POLLERR, "`epoll` uses same flag values as `poll`");
static_assert(EPOLLHUP == POLLHUP, "`epoll` uses same flag values as `poll`");

static const int kEpollNew = -1;
static const int kEpollAdded = 1;
static const int kEpollDeleted = 2;

AsyncPoller::AsyncPoller(EventLoop* loop)
  : Poller(loop)
  , epollfd_(epoll_create1(EPOLL_CLOEXEC))
  , epoll_events_(kInitNumEvents) {
  if (epollfd_ < 0)
    TYRLOG_SYSFATAL << "AsyncPoller::AsyncPoller";
}

AsyncPoller::~AsyncPoller(void) {
  // TODO:
}

basic::Timestamp AsyncPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  // TODO:
  return basic::Timestamp::now();
}

void AsyncPoller::update_channel(Channel* channel) {
  // TODO:
}

void AsyncPoller::remove_channel(Channel* channel) {
  // TODO:
}

const char* AsyncPoller::operation_to_string(int op) {
  switch (op) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    return "Unknown Operation";
  }
}

void AsyncPoller::update(int operation, Channel* channel) {
  // TODO:
}

void AsyncPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  // TODO:
}

}}
