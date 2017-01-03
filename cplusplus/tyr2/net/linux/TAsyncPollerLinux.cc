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

AsyncPoller::AsyncPoller(EventLoop* loop)
  : Poller(loop)
  , epollfd_(epoll_create1(EPOLL_CLOEXEC))
  , epoll_events_(kInitNumEvents) {
  if (epollfd_ < 0)
    TYRLOG_SYSFATAL << "AsyncPoller::AsyncPoller";
}

AsyncPoller::~AsyncPoller(void) {
  close(epollfd_);
}

basic::Timestamp AsyncPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  TYRLOG_TRACE << "AsyncPoller::poll - fd total count " << channels_.size();

  int events_count = static_cast<int>(epoll_events_.size());
  int num_events = epoll_wait(epollfd_, &*epoll_events_.begin(), events_count, timeout);
  int saved_errno = errno;
  if (num_events > 0) {
    TYRLOG_TRACE << "AsyncPoller::poll - " << num_events << " events happened";
    fill_active_channels(num_events, active_channels);
    if (num_events == events_count)
      epoll_events_.resize(events_count * 2);
  }
  else if (num_events == 0) {
    TYRLOG_TRACE << "AsyncPoller::poll - nothing happened";
  }
  else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      TYRLOG_SYSERR << "AsyncPoller::poll - errno=" << saved_errno;
    }
  }
  return basic::Timestamp::now();
}

void AsyncPoller::update_channel(Channel* channel) {
  assert_in_loopthread();

  int fd = channel->get_fd();
  const int index = channel->get_index();
  TYRLOG_TRACE << "AsyncPoller::update_channel - fd=" << fd
    << " index=" << index << " events=" << channel->get_events();

  if (index == POLLER_EVENT_NEW || index == POLLER_EVENT_DEL) {
    if (index == POLLER_EVENT_NEW) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    }
    else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(POLLER_EVENT_ADD);
    update(EPOLL_CTL_ADD, channel);
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == POLLER_EVENT_ADD);
    if (channel->is_none_event()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(POLLER_EVENT_DEL);
    }
    else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void AsyncPoller::remove_channel(Channel* channel) {
  assert_in_loopthread();

  int fd = channel->get_fd();
  const int index = channel->get_index();
  TYRLOG_TRACE << "AsyncPoller::remove_channel - fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());
  assert(index == POLLER_EVENT_ADD || index == POLLER_EVENT_DEL);
  size_t n = channels_.erase(fd);
  assert(n == 1); UNUSED(n);

  if (index == POLLER_EVENT_ADD)
    update(EPOLL_CTL_DEL,  channel);
  channel->set_index(POLLER_EVENT_NEW);
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
  int fd = channel->get_fd();
  TYRLOG_TRACE << "AsyncPoller::update - epoll_ctl op=" << operation_to_string(operation)
    << " fd=" << fd << " event={" << channel->events_to_string() << "}";

  struct epoll_event event = {0};
  event.events = channel->get_events();
  event.data.ptr = channel;
  if (epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL)
      TYRLOG_SYSERR << "AsyncPoller::update - epoll_ctl op=" << operation_to_string(operation) << " fd=" << fd;
    else
      TYRLOG_SYSFATAL << "AsyncPoller::update - epoll_ctl op=" << operation_to_string(operation) << " fd=" << fd;
  }
}

void AsyncPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  assert(basic::implicit_cast<size_t>(nevents) <= epoll_events_.size());
  for (auto i = 0; i < num_events; ++i) {
    Channel* channel = static_cast<Channel*>(epoll_events_[i].data.ptr);
    channel->set_revents(epoll_events_[i].events);
    active_channels->push_back(channel);
  }
}

}}
