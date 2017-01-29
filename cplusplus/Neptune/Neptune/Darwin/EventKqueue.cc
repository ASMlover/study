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

enum KeventOperation {
  KEVENT_ADD,
  KEVENT_DEL,
  KEVENT_MOD,
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
  CHAOS_UNUSED(timeout), CHAOS_UNUSED(active_channels);
  // TODO:
  return Chaos::Timestamp::now();
}

void EventKqueue::update_channel(Channel* channel) {
  CHAOS_UNUSED(channel);
  // TODO:
}

void EventKqueue::remove_channel(Channel* channel) {
  CHAOS_UNUSED(channel);
  // TODO:
}

void EventKqueue::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  CHAOS_UNUSED(nevents), CHAOS_UNUSED(active_channels);
  // TODO:
}

void EventKqueue::update(int operation, Channel* channel) {
  CHAOS_UNUSED(operation), CHAOS_UNUSED(channel);
  // TODO:
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
  case KEVENT_ADD:
    return kqueue_event_add(fd, channel);
  case KEVENT_DEL:
    return kqueue_event_del(fd);
  case KEVENT_MOD:
    return kqueue_event_mod(fd, channel);
  }
  return 0;
}

const char* EventKqueue::operation_to_string(int operation) {
  switch (operation) {
  case KEVENT_ADD:
    return "ADD";
  case KEVENT_DEL:
    return "DEL";
  case KEVENT_MOD:
    return "MOD";
  }
  return "Unknown Operation";
}

}
