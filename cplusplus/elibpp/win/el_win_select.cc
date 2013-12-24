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
#include "../el_net_internal.h"
#include "../el_socket.h"
#include "../el_connector.h"
#include "el_win_select.h"



namespace el {


struct win_fd_set {
  u_int fd_count;
  int fd_array[1];
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
#define WINFD_CLR     FD_CLR
#define WINFD_ZERO    FD_ZERO
#define WINFD_ISSET   FD_ISSET
#define WINFD_COPY(dest, src) do {\
  ((win_fd_set*)(dest))->fd_count = ((win_fd_set*)(src))->fd_count;\
  memcpy(((win_fd_set*)(dest))->fd_array, \
      ((win_fd_set*)(src))->fd_array, \
      ((win_fd_set*)(src))->fd_count * sizeof(int));\
} while (0)



struct SelectEntry {
  int fd;
  Connector* conn;
};



Select::Select(void)
  : fd_storage_(FD_SETSIZE)
  , rset_in_(NULL)
  , wset_in_(NULL)
  , rset_out_(NULL)
  , wset_out_(NULL)
  , has_removed_(false)
{
  if (!Init())
    abort();
}

Select::~Select(void)
{
  Destroy();
}

bool 
Select::Init(void)
{
  fd_storage_ = FD_SETSIZE;
  size_t size = sizeof(win_fd_set) + (fd_storage_ - 1) * sizeof(int);

  if (NULL == (rset_in_ = (win_fd_set*)malloc(size)))
    return false;

  do {
    if (NULL == (wset_in_ = (win_fd_set*)malloc(size)))
      break;

    if (NULL == (rset_out_ = (win_fd_set*)malloc(size)))
      break;
    if (NULL == (wset_out_ = (win_fd_set*)malloc(size)))
      break;

    WINFD_ZERO(rset_in_);
    WINFD_ZERO(wset_in_);

    entry_list_.clear();

    return true;
  } while (0);

  Destroy();
  return false;
}

void 
Select::Destroy(void)
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

  entry_list_.clear();
}

bool 
Select::Regrow(void)
{
  uint32_t new_fd_storage = (0 != fd_storage_ ? 
      2 * fd_storage_ : FD_SETSIZE);
  size_t size = sizeof(win_fd_set) + (new_fd_storage - 1) * sizeof(int);

  if (NULL == (rset_in_ = (win_fd_set*)realloc(rset_in_, size)))
    return false;

  do {
    if (NULL == (wset_in_ = (win_fd_set*)realloc(wset_in_, size)))
      break;

    if (NULL == (rset_out_ = (win_fd_set*)realloc(rset_out_, size)))
      break;
    if (NULL == (wset_out_ = (win_fd_set*)realloc(wset_out_, size)))
      break;

    fd_storage_ = new_fd_storage;

    return true;
  } while (0);

  Destroy();
  return false;
}


bool 
Select::Insert(Connector* conn)
{
  if (NULL == conn)
    return false;

  if (entry_list_.size() + 1 > fd_storage_) {
    if (!Regrow())
      return false;
  }

  SelectEntry entry = {conn->fd(), conn};
  entry_list_.push_back(entry);

  return true;
}

void 
Select::Remove(Connector* conn)
{
  if (NULL == conn)
    return;

  int fd =  conn->fd();
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
Select::AddEvent(Connector* conn, int ev)
{
  if (NULL == conn)
    return false;

  int fd = conn->fd();

  if (ev & kEventTypeRead)
    WINFD_SET(fd, rset_in_);

  if (ev & kEventTypeWrite)
    WINFD_SET(fd, wset_in_);

  return true;
}

bool 
Select::DelEvent(Connector* conn, int ev)
{
  if (NULL == conn)
    return false;

  int fd = conn->fd();

  if (ev & kEventTypeRead)
    WINFD_CLR(fd, rset_in_);

  if (ev & kEventTypeWrite)
    WINFD_CLR(fd, wset_in_);

  return true;
}


struct HasRemoved {
  bool operator ()(const SelectEntry& entry) const 
  {
    return (kNetTypeInval == entry.fd);
  }
};

bool 
Select::Dispatch(Dispatcher* dispatcher, uint32_t millitm)
{
  if (NULL == dispatcher)
    return false;

  struct timeval tv = {millitm / 1000, (millitm % 1000) * 1000};

  WINFD_COPY(rset_out_, rset_in_);
  WINFD_COPY(wset_out_, wset_in_);
  
  int ret = select(0, 
      (struct fd_set*)rset_out_, 
      (struct fd_set*)wset_out_, NULL, &tv);
  if (kNetTypeError == ret || 0 == ret)
    return false;

  size_t entry_count = entry_list_.size();
  SelectEntry* entry;
  for (size_t i = 0; i < entry_count; ++i) {
    entry = &entry_list_[i];

    if (kNetTypeInval == entry->fd)
      continue;
    if (WINFD_ISSET(entry->fd, rset_out_))
      dispatcher->DispatchReader(this, entry->conn);

    if (kNetTypeInval == entry->fd)
      continue;
    if (WINFD_ISSET(entry->fd, wset_out_))
      dispatcher->DispatchWriter(this, entry->conn);
  }

  if (has_removed_) {
    entry_list_.erase(std::remove_if(entry_list_.begin(), 
          entry_list_.end(), HasRemoved()), entry_list_.end());
    has_removed_ = false;
  }

  return true;
}


}
