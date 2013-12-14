//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "libnet_internal.h"
#include <unistd.h>
#include <stdlib.h>
#include "logging.h"
#include "socket.h"
#include "connector.h"
#include "epoll.h"



Epoll::Epoll(void)
  : fd_(kNetTypeInval)
  , event_count_(kEventCount)
  , events_(NULL)
{
  if (!Init())
    LOG_FAIL("Epoll::Epoll failed\n");
}

Epoll::~Epoll(void)
{
  Destroy();
}

bool 
Epoll::Init(void)
{
  fd_ = epoll_create(kEpollSize);
  if (kNetTypeInval == fd_)
    return false;

  event_count_ = kEventCount;
  size_t size = sizeof(struct epoll_event) * event_count_;

  do {
    events_ = (struct epoll_event*)malloc(size);
    if (NULL == events_)
      break;

    return true;
  } while (0);

  Destroy();
  return false;
}

void 
Epoll::Destroy(void)
{
  if (NULL != events_)  {
    free(events_);
    events_ = NULL;
  }
  event_count_ = kEventCount;

  if (kNetTypeInval != fd_) {
    close(fd_);
    fd_ = kNetTypeInval;
  }
}

bool 
Epoll::Regrow(void)
{
  uint32_t new_event_count = (0 != event_count_ ? 
      2 * event_count_ : kEventCount);
  size_t size = sizeof(struct epoll_event) * new_event_count;

  events_ = (struct epoll_event*)realloc(events_, size);
  if (NULL == events_) {
    LOG_FAIL("Epoll::Regrow failed\n");
    return false;
  }

  event_count_ = new_event_count;

  return true;
}


bool 
Epoll::Insert(Connector* conn)
{
  if (NULL == conn)
    return false;

  int fd = conn->fd();
  struct epoll_event ev;
  ev.events = 0;
  ev.data.ptr = conn;

  int ret = epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
  if (kNetTypeError == ret)
    return false;

  conn->set_events(EPOLLET);

  return true;
}

void 
Epoll::Remove(Connector* conn)
{
  if (NULL == conn)
    return;

  int fd = conn->fd();
  struct epoll_event ev;
  ev.events = conn->events();
  ev.data.ptr = conn;

  epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &ev);
}

bool 
Epoll::AddEvent(Connector* conn, int ev)
{
  if (NULL == conn)
    return false;

  int fd = conn->fd();
  struct epoll_event event;
  event.events = conn->events();
  event.data.ptr = conn;

  if (ev & kEventTypeRead)
    event.events |= EPOLLIN;

  if (ev & kEventTypeWrite)
    event.events |= EPOLLOUT;

  if (kNetTypeError == epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event))
    return false;
  conn->set_events(event.events);

  return true;
}

bool 
Epoll::DelEvent(Connector* conn, int ev)
{
  if (NULL == conn)
    return false;

  int fd = conn->fd();
  struct epoll_event event;
  event.events = conn->events();
  event.data.ptr = conn;

  if (ev & kEventTypeRead)
    event.events &= ~EPOLLIN;

  if (ev & kEventTypeWrite)
    event.events &= ~EPOLLOUT;

  if (kNetTypeError == epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event))
    return false;
  conn->set_events(event.events);

  return true;
}

bool 
Epoll::Dispatch(Dispatcher* dispatcher, int millitm)
{
  if (NULL == dispatcher)
    return false;

  int n = epoll_wait(fd_, events_, event_count_, millitm);
  if (kNetTypeError == n || 0 == n)
    return false;

  Connector* conn;
  for (int i = 0; i < n; ++i) {
    conn = (Connector*)events_[i].data.ptr;
    if (NULL == conn)
      continue;

    if (kNetTypeInval == conn->fd())
      continue;
    if (events_[i].events & EPOLLIN)
      dispatcher->DispatchReader(this, conn);

    if (kNetTypeInval == conn->fd())
      continue;
    if (events_[i].events & EPOLLOUT)
      dispatcher->DispatchWriter(this, conn);
  }

  if ((uint32_t)n == event_count_)
    Regrow();

  return true;
}
