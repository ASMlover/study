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
#include <algorithm>
#include "../basic/TTypes.h"
#include "../basic/TLogging.h"
#include "TChannel.h"
#include "TPollPoller.h"

namespace tyr { namespace net {

PollPoller::PollPoller(EventLoop* loop)
  : Poller(loop) {
}

PollPoller::~PollPoller(void) {
}

basic::Timestamp PollPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  int num_events = SocketSupport::kern_poll(&*pollfds_.begin(), pollfds_.size(), timeout);
  int saved_errno = errno;

  if (num_events > 0) {
    TYRLOG_TRACE << "PollPoller::poll - " << num_events << " events happended";
    fill_active_channels(num_events, active_channels);
  }
  else if (0 == num_events) {
    TYRLOG_TRACE << "PollPoller::poll - nothing happended";
  }
  else {
    if (EINTR != saved_errno) {
      errno = saved_errno;
      TYRLOG_TRACE << "PollPoller::poll - errno=" << saved_errno;
    }
  }

  return basic::Timestamp::now();
}

void PollPoller::update_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "PollPoller::update_channel - fd=" << channel->get_fd() << ", events=" << channel->get_events();
  if (channel->get_index() < 0) {
    // add new fd into pollfds_
    assert(channels_.find(channel->get_fd()) == channels_.end());
    KernPollfd pfd;
    pfd.fd = channel->get_fd();
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    channel->set_index(static_cast<int>(pollfds_.size()) - 1);
    channels_[pfd.fd] = channel;
  }
  else {
    // updated the events of an existing channel
    assert(channels_.find(channel->get_fd()) != channels_.end());
    assert(channels_[channel->get_fd()] == channel);
    int index = channel->get_index();
    assert(0 <= index && index < static_cast<int>(pollfds_.size()));
    KernPollfd& pfd = pollfds_[index];
    assert(pfd.fd == channel->get_fd() || pfd.fd == -channel->get_fd() - 1);
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    if (channel->is_none_event())
      pfd.fd = -channel->get_fd() - 1;
  }
}

void PollPoller::remove_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "PollPoller::remove_channel - fd=" << channel->get_fd();
  assert(channels_.find(channel->get_fd()) != channels_.end());
  assert(channels_[channel->get_fd()] == channel);
  assert(channel->is_none_event());
  int index = channel->get_index();
  assert(0 <= index && index < static_cast<int>(pollfds_.size()));
  const auto& pfd = pollfds_[index]; UNUSED(pfd);
  assert(pfd.fd == -channel->get_fd() - 1 && pfd.events == channel->get_events());
  size_t n = channels_.erase(channel->get_fd());
  assert(n == 1); UNUSED(n);
  if (basic::implicit_cast<size_t>(index) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  }
  else {
    int channel_end_fd = pollfds_.back().fd;
    std::iter_swap(pollfds_.begin() + index, pollfds_.end() - 1);
    if (channel_end_fd < 0)
      channel_end_fd = -channel_end_fd - 1;
    channels_[channel_end_fd]->set_index(index);
    pollfds_.pop_back();
  }
}

void PollPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  for (const auto& pfd : pollfds_) {
    if (nevents <= 0)
      break;

    if (pfd.revents > 0) {
      --nevents;
      const auto ch = channels_.find(pfd.fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->get_fd() == pfd.fd);
      channel->set_revents(pfd.revents);

      active_channels->push_back(channel);
    }
  }
}

}}
