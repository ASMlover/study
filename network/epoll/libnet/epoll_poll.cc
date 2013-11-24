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
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include "net.h"
#include "socket.h"
#include "epoll_poll.h"



#define OTHER_EVENTS  (EPOLLERR | EPOLLHUP | EPOLLET)
#define ALL_EVENTS    (kEventTypeRead | kEventTypeWrite | OTHER_EVENTS)




EpollPoll::EpollPoll(void)
  : fd_(kNetTypeInval)
  , rbytes_(kDefaultBufferLen)
  , wbytes_(kDefaultBufferLen)
  , event_count_(kDefaultEventCount)
  , events_(NULL)
  , handler_(NULL)
  , spinlock_()
{
  connectors_.clear();
  if (!Init())
    abort();
}

EpollPoll::~EpollPoll(void)
{
  CloseAll();
  Destroy();
}

bool 
EpollPoll::Init(void)
{
  fd_ = epoll_create(event_count_);
  if (kNetTypeInval == fd_)
    return false;

  events_ = (epoll_event*)malloc(sizeof(epoll_event) * event_count_);
  if (NULL == events_) {
    close(fd_);
    return false;
  }

  return true;
}

void 
EpollPoll::Destroy(void)
{
  if (NULL != events_) {
    free(events_);
    events_ = NULL;
  }

  if (kNetTypeInval != fd_) {
    close(fd_);
    fd_ = kNetTypeInval;
  }
}

bool 
EpollPoll::UpdateEvent(int op, Socket* s, int ev)
{
  if (kNetTypeInval == fd_ || NULL == s)
    return false;

  struct epoll_event event;
  event.data.ptr = s;
  event.events = ev;
  return (kNetTypeError != epoll_ctl(fd_, op, s->fd(), &event));
}

void 
EpollPoll::CloseAll(void)
{
  if (NULL == handler_)
    return;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Socket*>::iterator it;
  for (it = connectors_.begin(); it != connectors_.end(); ++it) {
    if (NULL != it->second) {
      handler_->CloseEvent(it->second);
      UpdateEvent(EPOLL_CTL_DEL, it->second, ALL_EVENTS);
      it->second->Close();
      delete it->second;
    }
  }
  connectors_.clear();
}

bool 
EpollPoll::Insert(int fd, int ev)
{
  if (kNetTypeInval == fd_)
    return false;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Socket*>::iterator it = connectors_.find(fd);
  if (it == connectors_.end()) {
    Socket* s = new Socket();
    if (NULL == s)
      return false;

    s->Attach(fd);
    s->SetNonBlock();
    s->SetTcpNoDelay();
    s->SetKeepAlive();
    s->SetSelfReadBuffer(rbytes_);
    s->SetSelfWriteBuffer(wbytes_);


    int events = ev | OTHER_EVENTS;
    if (!UpdateEvent(EPOLL_CTL_ADD, s, events)) {
      s->Close();
      delete s;
      return false;
    }

    connectors_[fd] = s;
  }

  return true;
}

void 
EpollPoll::Remove(int fd)
{
  if (NULL == handler_ || kNetTypeInval == fd_)
    return;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Socket*>::iterator it = connectors_.find(fd);
  if (it != connectors_.end()) {
    if (NULL != it->second) {
      handler_->CloseEvent(it->second);
      UpdateEvent(EPOLL_CTL_DEL, it->second, ALL_EVENTS);
      it->second->Close();
      delete it->second;
    }

    connectors_.erase(it);
  }
}

bool 
EpollPoll::AddEvent(int fd, int ev)
{
  if (kNetTypeInval == fd_)
    return false;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Socket*>::iterator it = connectors_.find(fd);
  if (it != connectors_.end()) {
    int events = kEventTypeRead | kEventTypeWrite | ev;
    if (!UpdateEvent(EPOLL_CTL_MOD, it->second, events))
      return false;
  }

  return true;
}

bool 
EpollPoll::DelEvent(int fd, int ev)
{
  if (kNetTypeInval == fd_)
    return false;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Socket*>::iterator it = connectors_.find(fd);
  if (it != connectors_.end()) {
    int events = (kEventTypeRead | kEventTypeWrite) & (~ev);
    if (!UpdateEvent(EPOLL_CTL_MOD, it->second, events))
      return false;
  }

  return true;
}

Socket* 
EpollPoll::GetConnector(int fd)
{
  Socket* s = NULL;

  {
    LockerGuard<SpinLock> guard(spinlock_);
    std::map<int, Socket*>::iterator it = connectors_.find(fd);
    if (it != connectors_.end())
      s = it->second;
  }

  return s;
}

bool 
EpollPoll::Polling(int ev, int millitm)
{
  return true;
}
