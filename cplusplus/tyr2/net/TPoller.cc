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
#include "../basic/TConfig.h"
#include "../basic/TLogging.h"
#include "TChannel.h"
#include "TPoller.h"

namespace tyr { namespace net {

Poller::Poller(EventLoop* loop)
  : owner_loop_(loop) {
}

Poller::~Poller(void) {
}

void Poller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  for (auto fdp = pollfds_.begin(); fdp != pollfds_.end() && nevents > 0; ++fdp) {
    if (nevents > 0) {
      --nevents;

      auto ch_iter = channels_.find(fdp->fd);
      assert(ch_iter != channels_.end());
      Channel* channel = ch_iter->second;
      assert(channel->get_fd() == fdp->fd);
      channel->set_revents(fdp->revents);
      active_channels->push_back(channel);
    }
  }
}

basic::Timestamp Poller::poll(int timeout, std::vector<Channel*>* active_channels) {
  basic::Timestamp now(basic::Timestamp::now());
  if (pollfds_.empty())
    return now;

  int nevents = SocketSupport::kern_poll(&*pollfds_.begin(), pollfds_.size(), timeout);
  if (nevents > 0) {
    TYRLOG_TRACE << "Poller::poll() - " << nevents << " events happened";
    fill_active_channels(nevents, active_channels);
  }
  else if (0 == nevents) {
    TYRLOG_TRACE << "Poller::poll() - nothing happened";
  }
  else {
    TYRLOG_SYSERR << "Poller::poll()";
  }

  return now;
}

void Poller::update_channel(Channel* channel) {
  assert_in_loopthread();
  TYRLOG_TRACE << "fd = " << channel->get_fd() << " events = " << channel->get_events();
  if (channel->get_index() < 0) {
    assert(channels_.find(channel->get_fd()) == channels_.end());
    KernPollfd pfd;
    pfd.fd = channel->get_fd();
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int index = static_cast<int>(pollfds_.size()) - 1;
    UNUSED(index);
    channels_[pfd.fd] = channel;
  }
  else {
    assert(channels_.find(channel->get_fd()) != channels_.end());
    assert(channels_[channel->get_fd()] == channel);
    int index = channel->get_index();
    assert(0 <= index && index < static_cast<int>(pollfds_.size()));
    KernPollfd& pfd = pollfds_[index];
    assert(pfd.fd == channel->get_fd() || pfd.fd == -1);
    pfd.events = static_cast<short>(channel->get_events());
    pfd.revents = 0;
    if (channel->is_none_event())
      pfd.fd = -1;
  }
}

}}
