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

EventEpoll::EventEpoll(EventLoop* loop)
  : Poller(loop)
  , epollfd_(epoll_create1(EPOLL_CLOEXEC))
  , epoll_events_(kInitEventsCount) {
  if (epollfd_ < 0)
    CHAOSLOG_SYSFATAL << "EventEpoll::EventEpoll - create fd of epoll failed";
}

EventEpoll::~EventEpoll(void) {
  close(epollfd_);
}

Chaos::Timestamp EventEpoll::poll(int timeout, std::vector<Channel*>& active_channels) {
  CHAOSLOG_TRACE << "EventEpoll::poll - fd total count is: " << channels_.size();

  int old_nevents = static_cast<int>(epoll_events_.size());
  int nevents = epoll_wait(epollfd_, &*epoll_events_.begin(), old_nevents, timeout);
  int saved_errno = errno;
  if (nevents > 0) {
    CHAOSLOG_TRACE << "EventEpoll::poll - " << nevents << " events happened";
    fill_active_channels(nevents, active_channels);
    if (old_nevents == nevents)
      epoll_events_.resize(old_nevents * 2);
  }
  else if (nevents == 0) {
    CHAOSLOG_TRACE << "EventEpoll::poll - nothing happened";
  }
  else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      CHAOSLOG_SYSERR << "EventEpoll::poll - errno=" << saved_errno;
    }
  }
  return Chaos::Timestamp::now();
}

void EventEpoll::update_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventEpoll::update_channel - fd=" << fd << " events=" << channel->get_events();

  if (index == Poller::EVENT_NEW || index == Poller::EVENT_DEL) {
    if (index == Poller::EVENT_NEW) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    }
    else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(Poller::EVENT_ADD);
    update(EPOLL_CTL_ADD, channel);
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == Poller::EVENT_ADD);
    if (channel->is_none_event()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(Poller::EVENT_DEL);
    }
    else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EventEpoll::remove_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventEpoll::remove_channel - fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());
  assert(index == Poller::EVENT_ADD || index == Poller::EVENT_DEL);

  if (index == Poller::EVENT_ADD)
    update(EPOLL_CTL_DEL, channel);
  channel->set_index(Poller::EVENT_NEW);
}

void EventEpoll::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  assert(Chaos::implicit_cast<std::size_t>(nevents) <= epoll_events_.size());
  for (int i = 0; i < nevents; ++i) {
    Channel* channel = static_cast<Channel*>(epoll_events_[i].data.ptr);
    channel->set_revents(epoll_events_[i].events);
    active_channels.push_back(channel);
  }
}

void EventEpoll::update(int operation, Channel* channel) {
  const int fd = channel->get_fd();
  CHAOSLOG_TRACE << "EventEpoll::update - operation=" << operation_to_string(operation)
    << " fd=" << fd << " events={" << channel->get_events() << "}";

  struct epoll_event event{};
  event.events = channel->get_events();
  event.data.ptr = channel;
  if (epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == Poller::EVENT_DEL)
      CHAOSLOG_SYSERR << "EventEpoll::update - operation=" << operation_to_string(operation) << " fd=" << fd;
    else
      CHAOSLOG_SYSFATAL << "EventEpoll::update - operation=" << operation_to_string(operation) << " fd=" << fd;
  }
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
