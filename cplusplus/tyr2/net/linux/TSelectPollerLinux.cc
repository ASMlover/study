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
#include <sys/types.h>
#include "../../basic/TTypes.h"
#include "../../basic/TLogging.h"
#include "../TSocketSupport.h"
#include "../TChannel.h"
#include "TSelectPollerLinux.h"

namespace tyr { namespace net {

SelectPoller::SelectPoller(EventLoop* loop)
  : Poller(loop) {
}

SelectPoller::~SelectPoller(void) {
}

basic::Timestamp SelectPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  struct timeval tv = {timeout / 1000, timeout % 1000 * 1000};
  memcpy(&rsets_out_, &rsets_in_, sizeof(rsets_in_));
  memcpy(&wsets_out_, &wsets_in_, sizeof(wsets_in_));

  int num_events = select(max_fd_ + 1, &rsets_out_, &wsets_out_, nullptr, &tv);
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
    assert(channels_.find(fd) == channels_.end());
    if (channels_.size() >= FD_SETSIZE)
      return;

    if (channel->is_reading())
      FD_SET(fd, &rsets_in_);
    if (channel->is_writing())
      FD_SET(fd, &wsets_in_);
    channel->set_index(fd);
    channels_[fd] = channel;

    if (fd > max_fd_)
      max_fd_ = fd;
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->get_index() > 0);
    if (!channel->is_reading())
      FD_CLR(fd, &rsets_in_);
    if (!channel->is_writing())
      FD_CLR(fd, &wsets_in_);
  }
}

void SelectPoller::remove_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "SelectPoller::remove_channel - fd=" << channel->get_fd();

  int fd = channel->get_fd();
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(Channel->get_index() > 0);

  FD_CLR(fd, &rsets_in_);
  FD_CLR(fd, &wsets_in_);
  FD_CLR(fd, &rsets_out_);
  FD_CLR(fd, &wsets_out_);

  if (fd == max_fd_) {
    max_fd_ = 0;
    for (ch : channels_) {
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
    if (FD_ISSET(channel->get_fd(), &rsets_out_))
      revents |= POLLIN;
    if (FD_ISSET(channel->get_fd(), &wsets_out_))
      revents |= POLLOUT;

    if (revents == 0)
      continue;

    --nevents;
    channel->set_revents(revents);
    active_channels->push_back(channel);
  }
}

}}
