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
#define _DARWIN_UNLIMITED_SELECT // allowed nfds greater than FD_SETSIZE

#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include "../../basic/TLogging.h"
#include "../TSocketSupport.h"
#include "../TChannel.h"
#include "TSelectPollerDarwin.h"

namespace tyr { namespace net {

#define DARWINFD_FREE(set) if (!(set)) { free(set); (set) = nullptr; }

static inline size_t calc_sets_bytes(int nfds) {
  return howmany(nfds, NFDBITS) * sizeof(fd_mask);
}

SelectPoller::FdsEntity::FdsEntity(int nsets) {
  if (resize(nsets)) {
    FD_ZERO(esets);
    FD_ZERO(rsets);
    FD_ZERO(wsets);
  }
}

SelectPoller::FdsEntity::~FdsEntity(void) {
  destroy();
}

void SelectPoller::FdsEntity::destroy(void) {
  DARWINFD_FREE(esets);
  DARWINFD_FREE(rsets);
  DARWINFD_FREE(wsets);
}

bool SelectPoller::FdsEntity::resize(int new_nsets) {
  size_t size = calc_sets_bytes(new_nsets);

  if (nullptr == (esets = (fd_set*)realloc(esets, size)))
    return false;
  do {
    if (nullptr == (rsets = (fd_set*)realloc(rsets, size)))
      break;
    if (nullptr == (wsets = (fd_set*)realloc(wsets, size)))
      break;

    fdsize = size;
    return true;
  } while (false);

  destroy();
  return false;
}

void SelectPoller::FdsEntity::copy(const FdsEntity& r) {
  memcpy(esets, r.esets, r.fdsize);
  memcpy(rsets, r.rsets, r.fdsize);
  memcpy(wsets, r.wsets, r.fdsize);
}

void SelectPoller::FdsEntity::remove(int fd) {
  FD_CLR(fd, esets);
  FD_CLR(fd, rsets);
  FD_CLR(fd, wsets);
}

SelectPoller::SelectPoller(EventLoop* loop)
  : Poller(loop)
  , fd_storage_(FD_SETSIZE)
  , sets_in_(fd_storage_)
  , sets_out_(fd_storage_) {
}

SelectPoller::~SelectPoller(void) {
}

basic::Timestamp SelectPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  struct timeval tv = {timeout / 1000, timeout % 1000 * 1000};

  sets_out_.copn(sets_in_);
  int num_events = select(max_fd_ + 1, sets_out_.rsets, sets_out_.wsets, sets_out_.esets, &tv);
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

  int fd = channel->get_fd();
  const int index = channel->get_index();
  TYRLOG_TRACE << "SelectPoller::update_channel - fd=" << fd << ", events=" << channel->get_events();

  if (index == POLLER_EVENT_NEW || index == POLLER_EVENT_DEL) {
    if (index == POLLER_EVENT_NEW) {
      assert(channels_.find(fd) == channels_.end());
      if (channels_.size() >= static_cast<size_t>(fd_storage_)) {
        int new_nsets = (0 != fd_storage_ ? static_cast<int>(1.5 * fd_storage_) : FD_SETSIZE);
        sets_in_.resize(new_nsets);
        sets_out_.resize(new_nsets);
        fd_storage_ = new_nsets;
      }
      channels_[fd] = channel;
    }
    else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    FD_SET(fd, sets_in_.esets);
    if (channel->is_reading())
      FD_SET(fd, sets_in_.rsets);
    if (channel->is_writing())
      FD_SET(fd, sets_in_.wsets);
    channel->set_index(POLLER_EVENT_ADD);

    if (fd > max_fd_)
      max_fd_ = fd;
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == POLLER_EVENT_ADD);
    if (channel->is_none_event()) {
      sets_in_.remove(fd);
      channel->set_index(POLLER_EVENT_DEL);
    }
    else {
      if (channel->is_reading()) {
        if (!FD_ISSET(fd, sets_in_.rsets))
          FD_SET(fd, sets_in_.rsets);
      }
      else {
        if (FD_ISSET(fd, sets_in_.rsets))
          FD_CLR(fd, sets_in_.rsets);
      }

      if (channel->is_writing()) {
        if (!FD_ISSET(fd, sets_in_.wsets))
          FD_SET(fd, sets_in_.wsets);
      }
      else {
        if (FD_ISSET(fd, sets_in_.wsets))
          FD_CLR(fd, sets_in_.wsets);
      }
    }
  }
}

void SelectPoller::remove_channel(Channel* channel) {
  assert_in_loopthread();

  int fd = channel->get_fd();
  const int index = channel->get_index();
  TYRLOG_TRACE << "SelectPoller::remove_channel - fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());
  assert(index == POLLER_EVENT_ADD || index == POLLER_EVENT_DEL);

  sets_in_.remove(fd);
  sets_out_.remove(fd);

  if (fd == max_fd_) {
    max_fd_ = 0;
    for (auto& ch : channels_) {
      if (ch.first > max_fd_)
        max_fd_ = ch.first;
    }
  }
}

void SelectPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  for (auto& ch : channels_) {
    if (nevents <= 0)
      break;

    int revents = 0;
    Channel* channel = ch.second;
    if (FD_ISSET(channel->get_fd(), sets_out_.esets))
      revents |= POLLERR;
    if (FD_ISSET(channel->get_fd(), sets_out_.rsets))
      revents |= POLLIN;
    if (FD_ISSET(channel->get_fd(), sets_out_.wsets))
      revents |= POLLOUT;

    if (revents == 0)
      continue;

    --nevents;
    channel->set_revents(revents);
    active_channels->push_back(channel);
  }
}

}}
