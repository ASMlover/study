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
#if defined(_WINDOWS_) ||  defined(_MSC_VER)
# ifndef _WINDOWS_
#   include <winsock2.h>
# endif
# define EWOULDBLOCK  WSAEWOULDBLOCK
# define NErrno()     WSAGetLastError()
#endif
#include "net.h"
#include "socket.h"
#include "select_poll.h"



SelectPoll::SelectPoll(void)
  : rbytes_(kDefaultBufferLen)
  , wbytes_(kDefaultBufferLen)
  , handler_(NULL)
  , spinlock_()
{
  connectors_.clear();
  FD_ZERO(&rset_);
  FD_ZERO(&wset_);
}

SelectPoll::~SelectPoll(void)
{
  CloseAll();
}

void 
SelectPoll::CloseAll(void)
{
  if (NULL == handler_)
    return;

  LockerGuard<SpinLock> gaurd(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  for (it = connectors_.begin(); it != connectors_.end(); ++it) {
    if (NULL != it->second.second) {
      handler_->CloseEvent(it->second.second);
      it->second.second->Close();
      delete it->second.second;
    }
  }
  connectors_.clear();
}

bool 
SelectPoll::Insert(int fd, int ev)
{
  if (NULL == handler_)
    return false;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
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

    connectors_[fd] = std::make_pair<int, Socket*>(ev, s);
  }

  return true;
}

void 
SelectPoll::Remove(int fd)
{
  if (NULL == handler_)
    return;

  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it != connectors_.end()) {
    if (NULL != it->second.second) {
      handler_->CloseEvent(it->second.second);
      it->second.second->Close();
      delete it->second.second;
    }

    connectors_.erase(it);
  }
}

bool 
SelectPoll::AddEvent(int fd, int ev)
{
  if (NULL == handler_)
    return false;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it != connectors_.end()) 
    it->second.first |= ev;

  return true;
}

bool 
SelectPoll::DelEvent(int fd, int ev)
{
  if (NULL == handler_)
    return false;

  LockerGuard<SpinLock> gaurd(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it != connectors_.end()) 
    it->second.first &= ~ev;

  return true;
}

Socket* 
SelectPoll::GetConnector(int fd)
{
  Socket* s = NULL;

  {
    LockerGuard<SpinLock> gaurd(spinlock_);
    std::map<int, std::pair<int, Socket*> >::iterator it;
    it = connectors_.find(fd);
    if (it != connectors_.end())
      s = it->second.second;
  }

  return s;
}

bool 
SelectPoll::Polling(int ev, int millitm)
{
  if (NULL == handler_)
    return false;

  struct timeval timeout;
  if (-1 == millitm) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;
  }
  else {
    timeout.tv_sec = millitm / 1000;
    timeout.tv_usec = (millitm % 1000 * 1000);
  }

  int max_fd = 0;
  int ret = 0;
  if (kEventTypeRead == ev) {
    InitSet(ev, &rset_, &max_fd);
    ret = select(max_fd + 1, &rset_, NULL, NULL, &timeout);
  }
  else if (kEventTypeWrite == ev) {
    InitSet(ev, &wset_, &max_fd);
    ret = select(max_fd + 1, NULL, &wset_, NULL, &timeout);
  }
  else {
    return false;
  }

  if (kNetTypeError == ret || 0 == ret)
    return false;

  if (!DispatchEvent(ev))
    return false;

  return true;
}

bool 
SelectPoll::InitSet(int ev, fd_set* set, int* max_fd)
{
  if (NULL == set || NULL == max_fd)
    return false;

  FD_ZERO(set);
  *max_fd = 0;

  LockerGuard<SpinLock> gaurd(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  for (it = connectors_.begin(); it != connectors_.end();) {
    if (NULL == it->second.second) {
      connectors_.erase(it++);
    }
    else if (kNetTypeInval == it->second.second->fd()) {
      delete it->second.second;
      connectors_.erase(it++);
    }
    else {
      if (it->second.first & ev)
        FD_SET(it->first, set);

      if (it->first > *max_fd)
        *max_fd = it->first;

      ++it;
    }
  }

  return true;
}

bool 
SelectPoll::DispatchEvent(int ev)
{
  if (NULL == handler_)
    return false;

  int fd;
  Socket* s;

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  for (it = connectors_.begin(); it !=  connectors_.end(); ++it) {
    fd = it->first;
    s = it->second.second;

    if (NULL == s)
      continue;

    if (kEventTypeRead == ev) {
      if (FD_ISSET(fd, &rset_)) {
        int read_bytes = s->DealWithAsyncRead();
        if (read_bytes > 0) {
          if (s->CheckValidMessageInReadBuffer())
            handler_->ReadEvent(s);
        }
        else if (0 == read_bytes) {
          handler_->CloseEvent(s);
          s->Close();
        }
        else {
          if (EWOULDBLOCK != NErrno()) {
            handler_->CloseEvent(s);
            s->Close();
          }
        }
      }
    }
    else if (kEventTypeWrite == ev) {
      if (FD_ISSET(fd, &wset_)) {
        int write_bytes = s->DealWithAsyncWrite();
        if (write_bytes > 0) {
          handler_->WriteEvent(s);
        }
        else if (write_bytes < 0) {
          if (EWOULDBLOCK != NErrno()) {
            handler_->CloseEvent(s);
            s->Close();
          }
        }
      }
    }
  }

  return true;
}
