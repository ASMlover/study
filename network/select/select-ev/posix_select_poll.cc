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
#include "select_ev.h"
#if defined(EV_WIN)
# ifndef _WINDOWS_
#   include <winsock2.h>
# endif
#elif defined(EV_POSIX)
# include <sys/types.h>
# include <sys/select.h>
#endif
#include <string.h>
#include <algorithm>
#include "socket.h"
#include "select_poll.h"



struct SelectEntry {
  int fd;
  Socket* s;
};



SelectPoll::SelectPoll(void)
  : max_fd_(kNetTypeInval)
  , has_removed_(false)
{
  FD_ZERO(&rset_in_);
  FD_ZERO(&wset_in_);
  FD_ZERO(&rset_out_);
  FD_ZERO(&wset_out_);

  entry_list_.clear();
}

SelectPoll::~SelectPoll(void)
{
}

bool 
SelectPoll::Insert(int fd, Socket* s)
{
  if (kNetTypeInval == fd || NULL == s)
    return false;

  if (entry_list_.size() + 1 > FD_SETSIZE)
    return false;

  SelectEntry entry = {fd, s};
  entry_list_.push_back(entry);

  if (fd > max_fd_)
    max_fd_ = fd;

  return true;
}

void 
SelectPoll::Remove(int fd)
{
  std::vector<SelectEntry>::iterator it;
  for (it = entry_list_.begin(); it != entry_list_.end(); ++it) {
    if (it->fd == fd)
      break;
  }

  if (it == entry_list_.end())
    return;
  it->fd = kNetTypeInval;
  has_removed_ = true;

  FD_CLR(fd, &rset_in_);
  FD_CLR(fd, &wset_in_);

  FD_CLR(fd, &rset_out_);
  FD_CLR(fd, &wset_out_);

  if (fd == max_fd_) {
    max_fd_ = kNetTypeInval;
    for (it = entry_list_.begin(); it != entry_list_.end(); ++it) {
      if (it->fd > max_fd_)
        max_fd_ = it->fd;
    }
  }
}

bool 
SelectPoll::AddEvent(int fd, int ev)
{
  if (kNetTypeInval == fd)
    return false;

  if (ev & kEventTypeRead)
    FD_SET(fd, &rset_in_);

  if (ev & kEventTypeWrite)
    FD_SET(fd, &wset_in_);

  return true;
}

bool 
SelectPoll::DelEvent(int fd, int ev)
{
  if (kNetTypeInval == fd)
    return false;

  if (ev & kEventTypeRead)
    FD_CLR(fd, &rset_in_);

  if (ev & kEventTypeWrite)
    FD_CLR(fd, &wset_in_);

  return true;
}

static inline bool 
HasSetRemoved(const SelectEntry& entry)
{
  return (kNetTypeInval == entry.fd);
}

bool 
SelectPoll::Dispatch(EventDispatcher* dispatcher, int millitm)
{
  if (NULL == dispatcher)
    return false;

  struct timeval tv;
  if (-1 == millitm) {
    tv.tv_sec = 0;
    tv.tv_usec = 500;
  }
  else {
    tv.tv_sec = millitm / 1000;
    tv.tv_usec = (millitm % 1000) * 1000;
  }

  memcpy(&rset_out_, &rset_in_, sizeof(rset_in_));
  memcpy(&wset_out_, &wset_in_, sizeof(wset_in_));

  int ret = select(max_fd_ + 1, &rset_out_, &wset_out_, NULL, &tv);
  if (kNetTypeError == ret || 0 == ret)
    return false;

  int size = (int)entry_list_.size();
  SelectEntry* entry;
  for (int i = 0; i < size; ++i) {
    entry = &entry_list_[i];

    if (kNetTypeInval == entry->fd)
      continue;
    if (FD_ISSET(entry->fd, &rset_out_)) {
      //! TODO:
      //! dispatch reader
    }

    if (kNetTypeInval == entry->fd)
      continue;
    if (FD_ISSET(entry->fd, &wset_out_)) {
      //! TODO:
      //! dispatch writer
    }
  }


  if (has_removed_) {
    entry_list_.erase(std::remove_if(entry_list_.begin(), 
          entry_list_.end(), HasSetRemoved), entry_list_.end());
    has_removed_ = false;
  }

  return true;
}
