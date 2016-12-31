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

SelectPoller::SelectPoller(EventLoop* loop)
  : Poller(loop) {
  sets_alloc();
}

SelectPoller::~SelectPoller(void) {
  sets_dealloc();
}

basic::Timestamp SelectPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  struct timeval tv = {timeout / 1000, timeout % 1000 * 1000};
  WINFD_COPY(rsets_out_, rsets_in_);
  WINFD_COPY(wsets_out_, wsets_in_);

  int num_events = select(0, (struct fd_set*)rsets_out_, (struct fd_set*)wsets_out_, nullptr, &tv);
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
  if (channel->get_index() < 0) {
    // add new fd events listening into channels_
    assert(channels_.find(channel->get_fd()) == channels_.end());
    if (channels_.size() >= static_cast<size_t>(fd_storage_))
      sets_realloc();

    if (channel->is_reading())
      WINFD_SET(channel->get_fd(), rsets_in_);
    if (channel->is_writing())
      WINFD_SET(channel->get_fd(), wsets_in_);
    channel->set_index(channel->get_fd());
    channels_[channel->get_fd()] = channel;
  }
  else {
    // updated the events of an existing channel
    assert(channels_.find(channel->get_fd()) != channels_.end());
    assert(channels_[channel->get_fd()] == channel);
    assert(channel->get_index() > 0);
    if (!channel->is_reading())
      WINFD_CLR(channel->get_fd(), rsets_in_);
    if (!channel->is_writing())
      WINFD_CLR(channel->get_fd(), wsets_in_);
  }
}

void SelectPoller::remove_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "SelectPoller::remove_channel - fd=" << channel->get_fd();
  assert(channels_.find(channel->get_fd()) != channels_.end());
  assert(channels_[channel->get_fd()] == channel);
  assert(channel->is_none_event());

  int fd = channel->get_fd();
  WINFD_CLR(fd, rsets_in_);
  WINFD_CLR(fd, wsets_in_);
  WINFD_CLR(fd, rsets_out_);
  WINFD_CLR(fd, wsets_out_);
}

void SelectPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  for (auto& ch : channels_) {
    if (nevents <= 0)
      break;

    int revents = 0;
    Channel* channel = ch.second;
    if (WINFD_ISSET(channel->get_fd(), rsets_out_))
      revents |= POLLIN;
    if (WINFD_ISSET(channel->get_fd(), wsets_out_))
      revents |= POLLOUT;

    if (revents == 0)
      continue;

    --nevents;
    channel->set_revents(revents);
    active_channels->push_back(channel);
  }
}

void SelectPoller::sets_alloc(void) {
  fd_storage_ = FD_SETSIZE;
  size_t size = sizeof(_FdSet_t) + (fd_storage_ - 1) * sizeof(int);

  if (nullptr == (rsets_in_ = (_FdSet_t*)malloc(size)))
    return;

  do {
    if (nullptr == (wsets_in_ = (_FdSet_t*)malloc(size)))
      break;
    if (nullptr == (rsets_out_ = (_FdSet_t*)malloc(size)))
      break;
    if (nullptr == (wsets_out_ = (_FdSet_t*)malloc(size)))
      break;

    WINFD_ZERO(rsets_in_);
    WINFD_ZERO(wsets_in_);
    return;
  } while (false);

  sets_dealloc();
}

void SelectPoller::sets_dealloc(void) {
  if (nullptr != rsets_in_) {
    free(rsets_in_);
    rsets_in_ = nullptr;
  }
  if (nullptr != wsets_in_) {
    free(wsets_in_);
    wsets_in_ = nullptr;
  }
  if (nullptr != rsets_out_) {
    free(rsets_out_);
    rsets_out_ = nullptr;
  }
  if (nullptr != wsets_out_) {
    free(wsets_out_);
    wsets_out_ = nullptr;
  }
  fd_storage_ = FD_SETSIZE;
}

void SelectPoller::sets_realloc(void) {
  int new_fd_storage = (0 != fd_storage_ ? static_cast<int>(1.5 * fd_storage_) : FD_SETSIZE);
  size_t size = sizeof(_FdSet_t) + (new_fd_storage - 1) * sizeof(int);

  if (nullptr == (rsets_in_ = (_FdSet_t*)realloc(rsets_in_, size)))
    return;

  do {
    if (nullptr == (wsets_in_ = (_FdSet_t*)realloc(wsets_in_, size)))
      break;
    if (nullptr == (rsets_out_ = (_FdSet_t*)realloc(rsets_out_, size)))
      break;
    if (nullptr == (wsets_out_ = (_FdSet_t*)realloc(wsets_out_, size)))
      break;

    fd_storage_ = new_fd_storage;
    return;
  } while (false);

  sets_dealloc();
}

}}
