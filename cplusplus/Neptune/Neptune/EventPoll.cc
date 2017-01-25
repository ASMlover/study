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
#include <algorithm>
#include <cerrno>
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

  int fd = channel->get_fd();
  CHAOSLOG_TRACE << "EventPoll::update_channel - fd=" << fd << ", events=" << channel->get_events();
  if (channel->get_index() < 0) {
    CHAOS_CHECK(channels_.find(fd) == channels_.end(), "fd not in channels");
    NetOps::Pollfd_t pfd;
    pfd.fd = fd;
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    channel->set_index(static_cast<int>(pollfds_.size()) - 1);
    channels_[fd] = channel;
  }
  else {
    CHAOS_CHECK(channels_.find(fd) != channels_.end(), "fd should in channels");
    CHAOS_CHECK(channels_[fd] == channel, "fd should in channels");
    int index = channel->get_index();
    CHAOS_CHECK(0 <= index && index < static_cast<int>(pollfds_.size()), "index should valid");
    NetOps::Pollfd_t& pfd = pollfds_[index];
    CHAOS_CHECK(pfd.fd == fd || pfd.fd == -fd - 1, "");
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    if (channel->is_none_event())
      pfd.fd = -channel->get_fd();
  }
}

void EventPoll::remove_channel(Channel* channel) {
  assert_in_loopthread();

  int fd = channel->get_fd();
  CHAOSLOG_TRACE << "EventPoll::remove_channel - fd=" << fd;
  CHAOS_CHECK(channels_.find(fd) != channels_.end(), "fd should not in channels");
  CHAOS_CHECK(channels_[fd] == channel, "channels_[fd] should equal to channel");
  CHAOS_CHECK(channel->is_none_event(), "channel should none event");

  int index = channel->get_index();
  CHAOS_CHECK(0 <= index && index < static_cast<int>(pollfds_.size()), "index of channel should valid");
  const NetOps::Pollfd_t& pfd = pollfds_[index];
  CHAOS_CHECK(pfd.fd == -fd - 1 && pfd.events == channel->get_events(), "pollfd should valid");
  std::size_t n = channels_.erase(fd);
  CHAOS_CHECK(n == 1, "remove channel count should be 1");
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
      CHAOS_CHECK(channel_pair != channels_.end(), "channel_pair with fd not in channels_");
      Channel* channel = channel_pair->second;
      CHAOS_CHECK(channel->get_fd() == pfd.fd, "channel's fd should equal to pfd.fd");
      channel->set_revents(pfd.revents);
      active_channels.push_back(channel);
    }
  }
}

}
