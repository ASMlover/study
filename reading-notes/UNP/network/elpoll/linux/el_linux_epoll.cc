// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "../el_poll.h"
#include "../el_internal.h"
#include "el_linux_epoll.h"

namespace el {

Epoll::Epoll(void)
  : epoll_fd_(EL_NETINVAL)
  , event_count_(EVENT_COUNT)
  , events_(nullptr) {
  EL_ASSERT(Init(), "linux epoll init failed ...");
}

Epoll::~Epoll(void) {
  Destroy();
}

bool Epoll::Init(void) {
  if (EL_NETINVAL == (epoll_fd_ = epoll_create(EPOLL_COUNT)))
    return false;

  event_count_ = EVENT_COUNT;
  size_t size = sizeof(struct epoll_event) * event_count_;

  do {
    if (nullptr == (events_ = (struct epoll_event*)malloc(size)))
      break;

    return true;
  } while (0);

  Destroy();
  return false;
}

void Epoll::Destroy(void) {
  if (nullptr != events_) {
    free(events_);
    events_ = nullptr;
  }
  event_count_ = EVENT_COUNT;

  if (EL_NETINVAL != epoll_fd_) {
    close(epoll_fd_);
    epoll_fd_ = EL_NETINVAL;
  }
}

bool Epoll::Regrow(void) {
  uint32_t new_event_count = (0 != event_count_ ?
      2 * event_count_ : EVENT_COUNT);
  size_t size = sizeof(struct epoll_event) * new_event_count;

  events_ = (struct epoll_event*)realloc(events_, size);
  EL_ASSERT(nullptr != events_, "epoll regrow failed ...");

  event_count_ = new_event_count;
  return true;
}

bool Epoll::Insert(Connector& c) {
  struct epoll_event event;
  event.events = 0;
  event.data.ptr = &c;

  if (EL_NETERR == epoll_ctl(epoll_fd_,
        EPOLL_CTL_ADD, c.fd(), &event))
    return false;

  // add EPOLLET into connector
  // TODO:
  return true;
}

void Epoll::Remove(Connector& c) {
  struct epoll_event event;
  event.events = c.events();
  event.data.ptr = &c;

  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, c.fd(), &event);
}

bool Epoll::AddEvent(Connector& c, EventType event) {
  struct epoll_event add_event;
  add_event.events = c.events();
  add_event.data.ptr = &c;

  if (event == EventType::EVENTTYPE_READ)
    add_event.events |= EPOLLIN;
  if (event == EventType::EVENTTYPE_WRITE)
    add_event.events |= EPOLLOUT;

  if (EL_NETERR == epoll_ctl(epoll_fd_,
        EPOLL_CTL_MOD, c.fd(), &add_event))
    return false;

  // add add_event.events into connector
  // TODO:
  return true;
}

bool Epoll::DelEvent(Connector& c, EventType event) {
  struct epoll_event del_event;
  del_event.events = c.events();
  del_event.data.ptr = &c;

  if (event == EventType::EVENTTYPE_READ)
    del_event.events &= ~EPOLLIN;
  if (event == EventType::EVENTTYPE_WRITE)
    del_event.events &= ~EPOLLOUT;

  if (EL_NETERR == epoll_ctl(epoll_fd_,
        EPOLL_CTL_MOD, c.fd(), &del_event))
    return false;

  // add del_event.events into connector
  // TODO:
  return true;
}

bool Epoll::Dispatch(Dispatcher& dispatcher, uint32_t timeout) {
  int num = epoll_wait(epoll_fd_, events_, event_count_, timeout);
  if (EL_NETERR == num || 0 == num)
    return false;

  Connector* c;
  for (auto i = 0; i < num; ++i) {
    c = static_cast<Connector*>(events_[i].data.ptr);
    if (nullptr == c)
      continue;

    if (EL_NETINVAL == c->fd())
      continue;
    if (events_[i].events & EPOLLIN)
      dispather.DispatchReader(*this, *c);

    if (EL_NETINVAL == c->fd())
      continue;
    if (events_[i].events & EPOLLOUT)
      dispatcher.DispatchWriter(*this, *c);
  }

  if (static_cast<uint32_t>(num) == event_count_)
    Regrow();

  return true;
}

}
