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
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/event.h>
#include "../../basic/TTypes.h"
#include "../../basic/TLogging.h"
#include "../TChannel.h"
#include "TAsyncPollerDarwin.h"

namespace tyr { namespace net {

enum AyncOperation {
  ASYNC_ADD,
  ASYNC_DEL,
  ASYNC_MOD,
};

AsyncPoller::AsyncPoller(EventLoop* loop)
  : Poller(loop)
  , kqueuefd_(kqueue())
  , kqueue_events_(kInitNumEvents) {
  if (kqueuefd_ < 0)
    TYRLOG_SYSFATAL << "AsyncPoller::AsyncPoller";
}

AsyncPoller::~AsyncPoller(void) {
  close(kqueuefd_);
}

basic::Timestamp AsyncPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  TYRLOG_TRACE << "AsyncPoller::poll - fd total count " << channels_.size();

  struct timespec ts = {timeout / 1000, timeout % 1000 * 1000000};
  int events_count = static_cast<int>(kqueue_events_.size());
  int num_events = kevent(kqueuefd_, nullptr, &*kqueue_events_.begin(), events_count, &ts);
  int saved_errno = errno;
  if (num_events > 0) {
    TYRLOG_TRACE << "AsyncPoller::poll - " << num_events << " events happened";
    fill_active_channels(num_events, active_channels);
    if (num_events == events_count)
      kqueue_events_.resize(events_count * 2);
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
    update(ASYNC_ADD, channel);
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == POLLER_EVENT_ADD);
    if (channel->is_none_event()) {
      update(ASYNC_DEL, channel);
      channel->set_index(POLLER_EVENT_DEL);
    }
    else {
      update(ASYNC_MOD, channel);
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
    update(ASYNC_DEL, channel);
  channel->set_index(POLLER_EVENT_NEW);
}

const char* AsyncPoller::operation_to_string(int op) {
  switch (op) {
  case ASYNC_ADD:
    return "ADD";
  case ASYNC_DEL:
    return "DEL";
  case ASYNC_MOD:
    return "MOD";
  default:
    return "Unknown Operation";
  }
}

void AsyncPoller::update(int operation, Channel* channel) {
  int fd = channel->get_fd();
  TYRLOG_TRACE << "AsyncPoller::update - kevent_ctl op=" << operation_to_string(operation)
    << " fd=" << fd << " events={" << channel->get_events() << "}";

  if (kevent_ctl(operation, fd, channel) < 0) {
    if (operation == ASYNC_DEL)
      TYRLOG_SYSERR << "AsyncPoller::update - kevent_ctl op=" << operation_to_string(operation) << " fd=" << fd;
    else
      TYRLOG_SYSFATAL << "AsyncPoller::update - kevent_ctl op=" << operation_to_string(operation) << " fd=" << fd;
  }
}

void AsyncPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  assert(basic::implicit_cast<size_t>(nevents) <= kqueue_events_.size());
  for (int i = 0; i < nevents; ++i) {
    Channel* channel = static_cast<Channel*>(kqueue_events_[i].udata);

    int revents = 0;
    if (kqueue_events_[i].flags & EV_ERROR)
      revents |= POLLERR;
    if (kqueue_events_[i].flags & EV_EOF)
      revents |= POLLRDHUP;
    if (kqueue_events_[i].filter == EVFILT_READ)
      revents |= POLLIN;
    if (kqueue_events_[i].filter == EVFILT_WRITE)
      revents |= POLLOUT;

    if (revents == 0)
      continue;

    channel->set_revents(revents);
    active_channels->push_back(channel);
  }
}

int AsyncPoller::kevent_add(int fd, Channel* channel) {
  struct kevent kev[2];
  int nchanges = 0;

  if (channel->is_reading())
    EV_SET(kev + nchanges++, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)channel);
  if (channel->is_writing())
    EV_SET(kev + nchanges++, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)channel);

  return kevent(kqueuefd_, kev, nchanges, nullptr, 0, nullptr);
}

int AsyncPoller::kevent_del(int fd) {
  struct kevent kev[2];

  EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

  return kevent(kqueuefd_, kev, 2, nullptr, 0, nullptr);
}

int AsyncPoller::kevent_mod(int fd, Channel* channel) {
  struct kevent kev[2];
  int nchanges = 0;

  if (channel->is_reading())
    EV_SET(kev + nchanges++, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)channel);
  else
    EV_SET(kev + nchanges++, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  if (channel->is_writing())
    EV_SET(kev + nchanges++, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)channel);
  else
    EV_SET(kev + nchanges++, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

  return kevent(kqueuefd_, kev, nchanges, nullptr, 0, nullptr);
}

int AsyncPoller::kevent_ctl(int operation, int fd, Channel* channel) {
  switch (operation) {
  case ASYNC_ADD:
    return kevent_add(fd, channel);
  case ASYNC_DEL:
    return kevent_del(fd);
  case ASYNC_MOD:
    return kevent_mod(fd, channel);
  }

  return 0;
}

}}
