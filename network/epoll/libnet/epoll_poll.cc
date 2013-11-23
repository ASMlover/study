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

void 
EpollPoll::CloseAll(void)
{
  if (NULL == handler_)
    return;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Socket*>::iterator it;
  for (it = connectors_.begin(); it != connectors_.end(); ++it) {
    if (NULL != it->second) {
      //! TODO:
      //! remove epoll event
      handler_->CloseEvent(it->second);
      it->second->Close();
      delete it->second;
    }
  }
  connectors_.clear();
}
