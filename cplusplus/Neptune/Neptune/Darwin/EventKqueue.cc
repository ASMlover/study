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
#include <sys/event.h>
#include <unistd.h>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Channel.h>
#include <Neptune/Darwin/EventKqueue.h>

namespace Neptune {

enum KQueueEventOperation {
  KQUEUEEVENT_ADD,
  KQUEUEEVENT_DEL,
  KQUEUEEVENT_MOD,
};

EventKqueue::EventKqueue(EventLoop* loop)
  : Poller(loop)
  , kqueuefd_(kqueue())
  , kqueue_events_(kInitEventsCount) {
  if (kqueuefd_ < 0)
    CHAOSLOG_SYSFATAL << "EventKqueue::EventKqueue - create kqueue fd failed";
}

EventKqueue::~EventKqueue(void) {
  close(kqueuefd_);
}

Chaos::Timestamp EventKqueue::poll(int timeout, std::vector<Channel*>& active_channels) {
  CHAOSLOG_TRACE << "EventKqueue::poll - fd total count is: " << channels_.size();

  struct timespec ts{timeout / 1000, timeout % 1000 * 1000000};
  int old_nevents = static_cast<int>(kqueue_events_.size());
  int nevents = kevent(kqueuefd_, nullptr, 0, &*kqueue_events_.begin(), old_nevents, &ts);
  int saved_errno = errno;
  if (nevents > 0) {
    CHAOSLOG_TRACE << "EventKqueue::poll - " << nevents << " events happened";
    fill_active_channels(nevents, active_channels);
    if (nevents == old_nevents)
      kqueue_events_.resize(old_nevents * 2);
  }
  else if (nevents == 0) {
    CHAOSLOG_TRACE << "EventKqueue::poll - nothing happened";
  }
  else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      CHAOSLOG_TRACE << "EventKqueue::poll - errno=" << saved_errno;
    }
  }
  return Chaos::Timestamp::now();
}

void EventKqueue::update_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventKqueue::update_channel - fd=" << fd << " events=" << channel->get_events();

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
    update(KQUEUEEVENT_ADD, channel);
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == Poller::EVENT_ADD);
    if (channel->is_none_event()) {
      update(KQUEUEEVENT_DEL, channel);
      channel->set_index(Poller::EVENT_DEL);
    }
    else {
      update(KQUEUEEVENT_MOD, channel);
    }
  }
}

void EventKqueue::remove_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_index();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventKqueue::remove_channel - fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());
  assert(index == Poller::EVENT_ADD || index == Poller::EVENT_DEL);
  std::size_t n = channels_.erase(fd);
  assert(n == 1); CHAOS_UNUSED(n);

  if (index == Poller::EVENT_ADD)
    update(KQUEUEEVENT_DEL, channel);
  channel->set_index(Poller::EVENT_NEW);
}

void EventKqueue::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  assert(Chaos::implicit_cast<std::size_t>(nevents) <= kqueue_events_.size());
  for (int i = 0; i < nevents; ++i) {
    Channel* channel = static_cast<Channel*>(kqueue_events_[i].udata);

    int revents = 0;
    if (kqueue_events_[i].filter == EVFILT_READ)
      revents |= POLLIN;
    if (kqueue_events_[i].filter == EVFILT_WRITE)
      revents |= POLLOUT;
    if (kqueue_events_[i].flags & EV_ERROR)
      revents |= POLLERR;
    if (kqueue_events_[i].flags & EV_EOF)
      revents |= POLLHUP;

    if (revents == 0)
      continue;

    channel->set_revents(revents);
    active_channels.push_back(channel);
  }
}

void EventKqueue::update(int operation, Channel* channel) {
  const int fd = channel->get_fd();
  CHAOSLOG_TRACE << "EventKqueue::update - opetation=" << operation_to_string(operation)
    << " fd=" << fd << " events={" << channel->get_events() << "}";

  if (kqueue_event_ctl(operation, fd, channel) < 0) {
    if (operation == KQUEUEEVENT_DEL)
      CHAOSLOG_SYSERR << "EventKqueue::update - operation=" << operation_to_string(operation) << " fd=" << fd;
    else
      CHAOSLOG_SYSFATAL << "EventKqueue::update - operation=" << operation_to_string(operation) << " fd=" << fd;
  }
}

int EventKqueue::kqueue_event_add(int fd, Channel* channel) {
  struct kevent kev[2];
  int nchanges = 0;

  if (channel->is_reading())
    EV_SET(kev + nchanges++, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)channel);
  if (channel->is_writing())
    EV_SET(kev + nchanges++, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)channel);
  return kevent(kqueuefd_, kev, nchanges, nullptr, 0, nullptr);
}

int EventKqueue::kqueue_event_del(int fd) {
  struct kevent kev[2];

  EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
  return kevent(kqueuefd_, kev, 2, nullptr, 0, nullptr);
}

int EventKqueue::kqueue_event_mod(int fd, Channel* channel) {
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

int EventKqueue::kqueue_event_ctl(int operation, int fd, Channel* channel) {
  switch (operation) {
  case KQUEUEEVENT_ADD:
    return kqueue_event_add(fd, channel);
  case KQUEUEEVENT_DEL:
    return kqueue_event_del(fd);
  case KQUEUEEVENT_MOD:
    return kqueue_event_mod(fd, channel);
  }
  return 0;
}

const char* EventKqueue::operation_to_string(int operation) {
  switch (operation) {
  case KQUEUEEVENT_ADD:
    return "ADD";
  case KQUEUEEVENT_DEL:
    return "DEL";
  case KQUEUEEVENT_MOD:
    return "MOD";
  }
  return "Unknown Operation";
}

}
