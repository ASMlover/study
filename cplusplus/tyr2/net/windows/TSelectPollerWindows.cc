// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <assert.h>
#include <errno.h>
#include "../../basic/TTypes.h"
#include "../../basic/TLogging.h"
#include "../TChannel.h"
#include "TSelectPollerWindows.h"

namespace tyr { namespace net {

struct _FdSet_t {
  u_int fd_count;
  int fd_array[1];
};

#define WINFD_FREE(set) if (!(set)) { free(set); (set) = nullptr; }

static inline void WINFD_CLR(int fd, _FdSet_t* set) {
  FD_CLR(fd, set);
}

static inline void WINFD_ZERO(_FdSet_t* set) {
  FD_ZERO(set);
}

static inline bool WINFD_ISSET(int fd, _FdSet_t* set) {
  return FD_ISSET(fd, set) != 0;
}

static inline void WINFD_SET(int fd, _FdSet_t* set)  {
  u_int i;
  for (i = 0; i < set->fd_count; ++i) {
    if (fd == set->fd_array[i])
      break;
  }

  if (i == set->fd_count) {
    set->fd_array[i] = fd;
    ++set->fd_count;
  }
}

static inline void WINFD_COPY(_FdSet_t* d_set, _FdSet_t* s_set) {
  d_set->fd_count = s_set->fd_count;
  memcpy(d_set->fd_array, s_set->fd_array, s_set->fd_count * sizeof(int));
}

SelectPoller::FdsEntity::FdsEntity(int nsets) {
  size_t size = sizeof(_FdSet_t) + (nsets - 1) * sizeof(int);
  if (nullptr == (esets = (_FdSet_t*)malloc(size)))
    return;

  do {
    if (nullptr == (rsets = (_FdSet_t*)malloc(size)))
      break;
    if (nullptr == (rsets = (_FdSet_t*)malloc(size)))
      break;

    return;
  } while (false);
  destroy();
}

SelectPoller::FdsEntity::~FdsEntity(void) {
  destroy();
}

void SelectPoller::FdsEntity::copy(const SelectPoller::FdsEntity& r) {
  WINFD_COPY(esets, r.esets);
  WINFD_COPY(rsets, r.rsets);
  WINFD_COPY(wsets, r.wsets);
}

void SelectPoller::FdsEntity::destroy(void) {
  WINFD_FREE(esets);
  WINFD_FREE(rsets);
  WINFD_FREE(wsets);
}

void SelectPoller::FdsEntity::resize(int new_nsets) {
  size_t size = sizeof(_FdSet_t) + (new_nsets - 1) * sizeof(int);

  if (nullptr == (esets = (_FdSet_t*)realloc(esets, size)))
    return;

  do {
    if (nullptr == (rsets = (_FdSet_t*)realloc(rsets, size)))
      break;
    if (nullptr == (wsets = (_FdSet_t*)realloc(wsets, size)))
      break;

    return;
  } while (false);
  destroy();
}

void SelectPoller::FdsEntity::remove(int fd) {
  WINFD_CLR(fd, esets);
  WINFD_CLR(fd, rsets);
  WINFD_CLR(fd, wsets);
}

SelectPoller::SelectPoller(EventLoop* loop)
  : Poller(loop)
  , sets_in_(fd_storage_)
  , sets_out_(fd_storage_) {
}

SelectPoller::~SelectPoller(void) {
}

basic::Timestamp SelectPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  struct timeval tv = {timeout / 1000, timeout % 1000 * 1000};

  sets_out_.copy(sets_in_);
  int num_events = select(0,
      (struct fd_set*)sets_out_.rsets,
      (struct fd_set*)sets_out_.wsets,
      (struct fd_set*)sets_out_.esets,
      &tv);
  int saved_errno = errno;
  if (num_events > 0) {
    TYRLOG_TRACE << "SelectPoller::poll - " << num_events << " events happened";
    fill_active_channels(num_events, active_channels);
  }
  else if (num_events == 0) {
    TYRLOG_TRACE << "SelectPoller::poll - nothing happened";
  }
  else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      TYRLOG_SYSERR << "SelectPoller::poll - errno=" << saved_errno;
    }
  }
  return basic::Timestamp::now();
}

void SelectPoller::update_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "SelectPoller::update_channel - fd=" << channel->get_fd() << " events=" << channel->get_events();

  int fd = channel->get_fd();
  if (channel->get_index() < 0) {
    // add new fd events listening into channels_
    assert(channels_.find(fd) == channels_.end());
    if (channels_.size() >= static_cast<size_t>(fd_storage_)) {
      int new_nsets = (0 != fd_storage_ ? static_cast<int>(1.5 * fd_storage_) : FD_SETSIZE);
      sets_in_.resize(new_nsets);
      sets_out_.resize(new_nsets);
      fd_storage_ = new_nsets;
    }

    WINFD_SET(fd, sets_in_.esets);
    if (channel->is_reading())
      WINFD_SET(fd, sets_in_.rsets);
    if (channel->is_writing())
      WINFD_SET(fd, sets_in_.wsets);
    channel->set_index(fd);
    channels_[fd] = channel;
  }
  else {
    // updated the events of an existing channel
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->get_index() > 0);
    if (!channel->is_reading())
      WINFD_CLR(fd, sets_in_.rsets);
    if (!channel->is_writing())
      WINFD_CLR(fd, sets_in_.wsets);
  }
}

void SelectPoller::remove_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "SelectPoller::remove_channel - fd=" << channel->get_fd();

  int fd = channel->get_fd();
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());

  sets_in_.remove(fd);
  sets_out_.remove(fd);
}

void SelectPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  for (auto& ch : channels_) {
    if (nevents <= 0)
      break;

    int revents = 0;
    Channel* channel = ch.second;
    if (WINFD_ISSET(channel->get_fd(), sets_out_.esets))
      revents |= POLLERR;
    if (WINFD_ISSET(channel->get_fd(), sets_out_.rsets))
      revents |= POLLIN;
    if (WINFD_ISSET(channel->get_fd(), sets_out_.wsets))
      revents |= POLLOUT;

    if (revents == 0)
      continue;

    --nevents;
    channel->set_revents(revents);
    active_channels->push_back(channel);
  }
}

}}
