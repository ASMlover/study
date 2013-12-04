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
#ifndef _WINDOWS_
# include <winsock2.h>
#endif
#include <string.h>
#include <algorithm>
#include "logging.h"
#include "socket.h"
#include "connector.h"
#include "win_select_poll.h"


struct win_fd_set {
  u_int fd_count;
  int fd_array[1];
};

struct SelectEntry {
  int fd;
  Connector* conn;
};



#define WINFD_SET(fd, set) do {\
  u_int __i;\
  for (__i = 0; __i < ((win_fd_set*)(set))->fd_count; ++__i) {\
    if ((fd) == ((win_fd_set*)(set))->fd_array[__i])\
      break;\
  }\
  if (__i == ((win_fd_set*)(set))->fd_count) {\
    ((win_fd_set*)(set))->fd_array[__i] = (fd);\
    ++((win_fd_set*)(set))->fd_count;\
  }\
} while (0)
#define WINFD_ZERO    FD_ZERO
#define WINFD_CLR     FD_CLR
#define WINFD_ISSET   FD_ISSET
#define WINFD_COPY(dest, src) do {\
  (dest)->fd_count = (src)->fd_count;\
  memcpy((dest)->fd_array, (src)->fd_array, (src)->fd_count * sizeof(int));\
} while (0)





SelectPoll::SelectPoll(void)
  : has_removed_(false)
  , fd_count_(FD_SETSIZE)
  , rset_in_(NULL)
  , wset_in_(NULL)
  , rset_out_(NULL)
  , wset_out_(NULL)
{
  if (!Init())
    LOG_FAIL("SelectPoll Init failed\n");
}

SelectPoll::~SelectPoll(void)
{
  Destroy();
}

bool 
SelectPoll::Init(void)
{
  fd_count_ = FD_SETSIZE;
  size_t set_size = sizeof(win_fd_set) + (fd_count_ - 1) * sizeof(int);

  rset_in_ = (win_fd_set*)malloc(set_size);
  if (NULL == rset_in_)
    return false;

  do {
    wset_in_ = (win_fd_set*)malloc(set_size);
    if (NULL == wset_in_)
      break;

    rset_out_ = (win_fd_set*)malloc(set_size);
    if (NULL == rset_out_)
      break;
    wset_out_ = (win_fd_set*)malloc(set_size);
    if (NULL == wset_out_)
      break;

    WINFD_ZERO(rset_in_);
    WINFD_ZERO(wset_in_);

    return true;
  } while (0);

  if (NULL != rset_out_) {
    free(rset_out_);
    rset_out_ = NULL;
  }
  if (NULL != wset_in_) {
    free(wset_in_);
    wset_in_ = NULL;
  }
  if (NULL != rset_in_) {
    free(rset_in_);
    rset_in_ = NULL;
  }

  return false;
}

void 
SelectPoll::Destroy(void)
{
  if (NULL != rset_in_) {
    free(rset_in_);
    rset_in_ = NULL;
  }
  if (NULL != wset_in_) {
    free(wset_in_);
    wset_in_ = NULL;
  }

  if (NULL != rset_out_) {
    free(rset_out_);
    rset_out_ = NULL;
  }
  if (NULL != wset_out_) {
    free(wset_out_);
    wset_out_ = NULL;
  }
}

bool 
SelectPoll::Regrow(void)
{
  int new_fd_count = (0 != fd_count_ ? 2 * fd_count_ : FD_SETSIZE);
  size_t set_size = sizeof(win_fd_set) + (new_fd_count - 1) * sizeof(int);

  rset_in_ = (win_fd_set*)realloc(rset_in_, set_size);
  if (NULL == rset_in_)
    return false;

  do {
    wset_in_ = (win_fd_set*)realloc(wset_in_, set_size);
    if (NULL == wset_in_)
      break;

    rset_out_ = (win_fd_set*)realloc(rset_out_, set_size);
    if (NULL == rset_out_)
      break;
    wset_out_ = (win_fd_set*)realloc(wset_out_, set_size);
    if (NULL == wset_out_)
      break;

    fd_count_ = new_fd_count;
    return true;
  } while (0);

  return false;
}



bool 
SelectPoll::Insert(int fd, Connector* conn)
{
  if (entry_list_.size() + 1 > (size_t)fd_count_) {
    if (!Regrow())
      return false;
  }

  SelectEntry entry = {fd, conn};
  entry_list_.push_back(entry);

  return true;
}

void 
SelectPoll::Remove(int fd)
{
  std::vector<SelectEntry>::iterator it;
  for (it = entry_list_.begin(); it != entry_list_.end(); ++it) {
    if (fd == it->fd)
      break;
  }

  if (it == entry_list_.end())
    return;
  it->fd = kNetTypeInval;
  has_removed_ = true;

  WINFD_CLR(fd, rset_in_);
  WINFD_CLR(fd, wset_in_);

  WINFD_CLR(fd, rset_out_);
  WINFD_CLR(fd, wset_out_);
}

bool 
SelectPoll::AddEvent(int fd, int ev)
{
  if (ev & kEventTypeRead)
    WINFD_SET(fd, rset_in_);

  if (ev & kEventTypeWrite)
    WINFD_SET(fd, wset_in_);

  return true;
}

bool 
SelectPoll::DelEvent(int fd, int ev)
{
  if (ev & kEventTypeRead)
    WINFD_CLR(fd, rset_in_);

  if (ev & kEventTypeWrite) 
    WINFD_CLR(fd, wset_in_);

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

  WINFD_COPY(rset_out_, rset_in_);
  WINFD_COPY(wset_out_, wset_in_);

  int ret = select(0, 
      (struct fd_set*)rset_out_, 
      (struct fd_set*)wset_out_, NULL, &tv);
  if (kNetTypeError == ret || 0 == ret)
    return false;
  
  int size = (int)entry_list_.size();
  SelectEntry* entry;
  for (int i = 0; i < size; ++i) {
    entry = &entry_list_[i];

    if (kNetTypeInval == entry->fd)
      continue;
    if (WINFD_ISSET(entry->fd, rset_out_))
      dispatcher->DispatchReader(entry->conn);

    if (kNetTypeInval == entry->fd)
      continue;
    if (WINFD_ISSET(entry->fd, wset_out_)) 
      dispatcher->DispatchWriter(entry->conn);
  }
  
  if (has_removed_) {
    entry_list_.erase(std::remove_if(entry_list_.begin(), 
          entry_list_.end(), HasSetRemoved), entry_list_.end());
    has_removed_ = false;
  }

  return true;
}
