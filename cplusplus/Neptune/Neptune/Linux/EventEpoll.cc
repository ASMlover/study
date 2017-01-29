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
#include <cassert>
#include <cerrno>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Channel.h>
#include <Neptune/Linux/EventEpoll.h>

namespace Neptune {

static_assert(EPOLLIN == POLLIN, "`epoll` uses same flag value as `poll`");
static_assert(EPOLLPRI == POLLPRI, "`epoll` uses same flag value as `poll`");
static_assert(EPOLLOUT == POLLOUT, "`epoll` uses same flag value as `poll`");
static_assert(EPOLLRDHUP == POLLRDHUP, "`epoll` uses same flag value as `poll`");
static_assert(EPOLLERR == POLLERR, "`epoll` uses same flag value as `poll`");
static_assert(EPOLLHUP == POLLHUP, "`epoll` uses same flag value as `poll`");

EventEpoll::EventEpoll(EventLoop* loop) {
  // TODO:
}

EventEpoll::~EventEpoll(void) {
  // TODO:
}

Chaos::Timestamp EventEpoll::poll(int timeout, std::vector<Channel*>& active_channels) {
  // TODO:
  return Chaos::Timestamp::now();
}

void EventEpoll::update_channel(Channel* channel) {
  // TODO:
}

void EventEpoll::remove_channel(Channel* channel) {
  // TODO:
}

void EventEpoll::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  // TODO:
}

void EventEpoll::update(int operation, Channel* channel) {
  // TODO:
}

const char* EventEpoll::operation_to_string(int operation) {
  switch (operation) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  }
  return "Unknown Operation";
}

}
