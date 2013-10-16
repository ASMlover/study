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
#ifndef _WINDOWS_
# include <winsock2.h>
#endif
#include "global.h"
#include "event_handler.h"
#include "socket.h"
#include "select.h"



Select::Select(void)
  : spinlock_()
{
  FD_ZERO(&rset_);
  FD_ZERO(&wset_);

  handlers_.clear();
}

Select::~Select(void)
{
}

void 
Select::Insert(EventHandler* eh, int ev)
{
  eh->event_ = ev;
  LockerGuard<SpinLock> guard(spinlock_);
  handlers_[eh->fd()] = eh;
}

void 
Select::Remove(EventHandler* eh) 
{
  LockerGuard<SpinLock> guard(spinlock_);

  std::map<int, EventHandler*>::iterator it = handlers_.find(eh->fd());
  if (it != handlers_.end()) {
    it->second->s_->Close();
    delete it->second->s_;
    delete it->second;
    handlers_.erase(it);
  }
}

void 
Select::AddEvent(EventHandler* eh, int ev)
{
  LockerGuard<SpinLock> guard(spinlock_);

  std::map<int, EventHandler*>::iterator it = handlers_.find(eh->fd());
  if (it != handlers_.end())
    it->second->event_ |= ev;
}

void 
Select::DelEvent(EventHandler* eh, int ev)
{
  LockerGuard<SpinLock> guard(spinlock_);

  std::map<int, EventHandler*>::iterator it = handlers_.find(eh->fd());
  if (it != handlers_.end())
    it->second->event_ &= ~ev;
}

void
Select::Poll(void)
{
  InitSets();

  int ret = select(0, &rset_, &wset_, NULL, NULL);
  if (SOCKET_ERROR == ret) {
    //LOG_ERR("select error err-code (%d)\n", WSAGetLastError());
    return;
  }

  if (0 == ret)
    return;

  DispatchEvent(&rset_, EventHandler::ET_READ);
  DispatchEvent(&wset_, EventHandler::ET_WRITE);
}




void 
Select::InitSets(void)
{
  FD_ZERO(&rset_);
  FD_ZERO(&wset_);

  std::map<int, EventHandler*>::iterator it;

  int fd;
  EventHandler* eh = NULL;
  for (it = handlers_.begin(); it != handlers_.end(); ++it) {
    eh = it->second;
    if (NULL == eh)
      continue;

    fd = eh->fd();

    if (eh->event_ & EventHandler::ET_READ)
      FD_SET(fd, &rset_);

    if (eh->event_ & EventHandler::ET_WRITE)
      FD_SET(fd, &wset_);
  }
}

void 
Select::DispatchEvent(fd_set* fds, int ev)
{
  int fd;
  for (int i = 0; i < (int)fds->fd_count; ++i) {
    fd = fds->fd_array[i];

    EventHandler* eh = NULL;
    {
      LockerGuard<SpinLock> guard(spinlock_);
      std::map<int, EventHandler*>::iterator it = handlers_.find(fd);
      if (it != handlers_.end())
        eh = it->second;
    }

    if (NULL == eh)
      continue;

    switch (ev) {
    case EventHandler::ET_READ:
      {
        if (!eh->ReadEvent())
          Remove(eh);
      }
      break;
    case EventHandler::ET_WRITE:
      {
        if (!eh->WriteEvent())
          Remove(eh);
      }
      break;
    }
  }
}
