// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <cassert>
#include <cerrno>
#include <algorithm>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Channel.h>
#include <Neptune/EventPoll.h>

namespace Neptune {

EventPoll::EventPoll(EventLoop* loop)
  : Poller(loop) {
}

EventPoll::~EventPoll(void) {
}

Chaos::Timestamp EventPoll::poll(int timeout, std::vector<Channel*>& active_channels) {
  int nevents = NetOps::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
  int saved_errno = errno;

  if (nevents > 0) {
    CHAOSLOG_TRACE << "EventPoll::poll - " << nevents << " events happened";
    fill_active_channels(nevents, active_channels);
  }
  else if (nevents == 0) {
    CHAOSLOG_TRACE << "EventPoll::poll - nothing happened";
  }
  else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      CHAOSLOG_TRACE << "EventPoll::poll - errno=" << saved_errno;
    }
  }

  return Chaos::Timestamp::now();
}

void EventPoll::update_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  CHAOSLOG_TRACE << "EventPoll::update_channel - fd=" << fd << ", events=" << channel->get_events();
  if (channel->get_index() < 0) {
    assert(channels_.find(fd) == channels_.end());
    NetOps::Pollfd_t pfd;
    pfd.fd = fd;
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    channel->set_index(static_cast<int>(pollfds_.size()) - 1);
    channels_[fd] = channel;
  }
  else {
    const int index = channel->get_index();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(0 <= index && index < static_cast<int>(pollfds_.size()));
    NetOps::Pollfd_t& pfd = pollfds_[index];
    assert(pfd.fd == fd || pfd.fd == -fd - 1);
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    if (channel->is_none_event())
      pfd.fd = -channel->get_fd();
  }
}

void EventPoll::remove_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventPoll::remove_channel - fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());
  assert(0 <= index && index < static_cast<int>(pollfds_.size()));

  const NetOps::Pollfd_t& pfd = pollfds_[index];
  assert(pfd.fd == -fd - 1 && pfd.events == channel->get_events()); CHAOS_UNUSED(pfd);
  std::size_t n = channels_.erase(fd);
  assert(n == 1); CHAOS_UNUSED(n);
  if (Chaos::implicit_cast<std::size_t>(index) == pollfds_.size() - 1) {
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

void EventPoll::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  for (const auto& pfd : pollfds_) {
    if (nevents <= 0)
      break;

    if (pfd.revents > 0) {
      --nevents;
      const auto channel_pair = channels_.find(pfd.fd);
      assert(channel_pair != channels_.end());
      Channel* channel = channel_pair->second;
      assert(channel->get_fd() == pfd.fd);
      channel->set_revents(pfd.revents);
      active_channels.push_back(channel);
    }
  }
}

}
