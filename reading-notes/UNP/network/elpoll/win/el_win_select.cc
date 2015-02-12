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
    ((win_fd_set*)(set))->fd_array[__i] = fd;\
    ++((win_fd_set*)(set))->fd_count;\
  }\
} while (0)
#define WINFD_CLR   FD_CLR
#define WINFD_ZERO  FD_ZERO
#define WINFD_ISSET FD_ISSET
#define WINFD_COPY(dest, src) do {\
  ((win_fd_set*)(dest))->fd_count = ((win_fd_set*)(sec))->fd_count;\
  memcpy(((win_fd_set*)(dest))->fd_array, \
      ((win_fd_set*)(src))->fd_array, \
      ((win_fd_set*)(src))->fd_count * sizeof(int));\
} while (0)

struct SelectEntity {
  int fd;
  Connector* c;
};

Select::Select(void)
  : fd_storage_(FD_SETSIZE)
  , rset_in_(nullptr)
  , wset_in_(nullptr)
  , rset_out_(nullptr)
  , wset_out_(nullptr)
  , removed_(false) {
  EL_ASSERT(Init(), "select init failed ...");
}

Select::~Select(void) {
  Destroy();
}

bool Select::Init(void) {
  fd_storage_ = FD_SETSIZE;
  size_t size = sizeof(win_fd_set) + (fd_storage_ - 1) * sizeof(int);

  if (nullptr == (rset_in_ = (win_fd_set*)malloc(size)))
    return false;
  do {
    if (nullptr == (wset_in_ = (win_fd_set*)malloc(size)))
      break;

    if (nullptr == (rset_out_ = (win_fd_set*)malloc(size)))
      break;
    if (nullptr == (wset_out_ = (win_fd_set*)malloc(size)))
      break;

    WINFD_ZERO(rset_in_);
    WINFD_ZERO(wset_in_);
    entity_list_.clear();

    return true;
  } while (0);

  Destroy();
  return false;
}

void Select::Destroy(void) {
#define FREE_WINFD(set) do {\
  if (nullptr != (set)) {\
    free((set));\
    (set) = nullptr;\
  }\
} while (0)

  FREE_WINFD(rset_in_);
  FREE_WINFD(wset_in_);
  FREE_WINFD(rset_out_);
  FREE_WINFD(wset_out_);

  entity_list_.clear();

#undef FREE_WINFD
}

bool Select::Regrow(void) {
  uint32_t new_fd_storage = (0 != fd_storage_ ?  
      2 * fd_storage_ : FD_SETSIZE);
  size_t size = sizeof(win_fd_set) + (new_fd_storage - 1) * sizeof(int);

  if (nullptr == (rset_in_ = (win_fd_set*)realloc(rset_in_, size)))
    return false;
  do {
    if (nullptr == (wset_in_ = (win_fd_set*)realloc(wset_in_, size)))
      break;

    if (nullptr == (rset_out_ = (win_fd_set*)realloc(rset_out_, size)))
      break;
    if (nullptr == (wset_out_ = (win_fd_set*)realloc(wset_out_, size)))
      break;

    fd_storage_ = new_fd_storage;
    return true;
  } while (0);

  Destroy();
  return false;
}

bool Select::Insert(Connector& c) {
  if (entity_list_.size() + 1 > fd_storage_) {
    if (!Regrow())
      return false;
  }

  int fd = c.fd();
  entity_list_[fd] = std::make_pair(fd, &c);

  return true;
}

void Select::Remove(Connector& c) {
  int fd = c.fd();
  auto entity = entity_list_.find(fd);
  if (entity_list_.end() == entity)
    return;

  entity->second.first = EL_NETINVAL;
  removed_ = true;

  WINFD_CLR(fd, rset_in_);
  WINFD_CLR(fd, wset_in_);
  WINFD_CLR(fd, rset_out_);
  WINFD_CLR(fd, wset_out_);
}

bool Select::AddEvent(Connector& c, EventType event) {
  int fd = c.fd();

  if (event == EventType::EVENTTYPE_READ)
    WINFD_SET(fd, rset_in_);

  if (event == EventType::EVENTTYPE_WRITE)
    WINFD_SET(fd, wset_in_);

  return true;
}

bool Select::DelEvent(Connector& c, EventType event) {
  int fd = c.fd();
  
  if (event == EventType::EVENTTYPE_READ)
    WINFD_CLR(fd, rset_in_);

  if (event == EventType::EVENTTYPE_WRITE)
    WINFD_CLR(fd, wset_in_);

  return true;
}

bool Select::Dispatch(Dispatcher& dispatcher, uint32_t timeout) {
  return true;
}

}
