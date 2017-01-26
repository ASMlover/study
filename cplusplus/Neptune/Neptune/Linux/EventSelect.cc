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
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/Channel.h>
#include <Neptune/Linux/EventSelect.h>

namespace Neptune {

EventSelect::FdsEntity::FdsEntity(void) {
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&error_fds);
}

EventSelect::FdsEntity::~FdsEntity(void) {
}

void EventSelect::FdsEntity::copy(const FdsEntity& r) {
  std::memcpy(&read_fds, &r.read_fds, sizeof(r.read_fds));
  std::memcpy(&write_fds, &r.write_fds, sizeof(r.write_fds));
  std::memcpy(&error_fds, &r.error_fds, sizeof(r.error_fds));
}

void EventSelect::FdsEntity::remove(int fd) {
  FD_CLR(fd, &read_fds);
  FD_CLR(fd, &write_fds);
  FD_CLR(fd, &error_fds);
}

EventSelect::EventSelect(EventLoop* loop)
  : Poller(loop) {
}

EventSelect::~EventSelect(void) {
}

Chaos::Timestamp EventSelect::poll(int timeout, std::vector<Channel*>& active_channels) {
  struct timeval tv{timeout / 1000, timeout % 1000 * 1000};

  fds_out_.copy(fds_in_);
  int nevents = select(max_fd_ + 1, &fds_out_.read_fds, &fds_out_.write_fds, &fds_out_.error_fds, &tv);
  int saved_errno = errno;
  if (nevents > 0) {
    CHAOSLOG_TRACE << "EventSelect::poll - " << nevents << " events happened";
    fill_active_channels(nevents, active_channels);
  }
  else if (nevents == 0) {
    CHAOSLOG_TRACE << "EventSelect::poll - nothing happened";
  }
  else {
    if (saved_errno != EINTR) {
      errno = saved_errno;
      CHAOSLOG_SYSERR << "EventSelect::poll - errno=" << saved_errno;
    }
  }
  return Chaos::Timestamp::now();
}

void EventSelect::update_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventSelect::update_channel - fd=" << fd << ", events=" << channel->get_events();

  if (index == Poller::EVENT_NEW || index == Poller::EVENT_DEL) {
    if (index == Poller::EVENT_NEW) {
      assert(channels_.find(fd) == channels_.end());
      if (channels_.size() >= FD_SETSIZE)
        return;
      channels_[fd] = channel;
    }
    else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    FD_SET(fd, &fds_in_.error_fds);
    if (channel->is_reading())
      FD_SET(fd, &fds_in_.read_fds);
    if (channel->is_writing())
      FD_SET(fd, &fds_in_.write_fds);
    channel->set_index(Poller::EVENT_ADD);

    if (fd > max_fd_)
      max_fd_ = fd;
  }
  else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == Poller::EVENT_ADD);
    if (channel->is_none_event()) {
      fds_in_.remove(fd);
      channel->set_index(Poller::EVENT_DEL);
    }
    else {
      if (channel->is_reading()) {
        if (!FD_ISSET(fd, &fds_in_.read_fds))
          FD_SET(fd, &fds_in_.read_fds);
      }
      else {
        if (FD_ISSET(fd, &fds_in_.read_fds))
          FD_CLR(fd, &fds_in_.read_fds);
      }

      if (channel->is_writing()) {
        if (!FD_ISSET(fd, &fds_in_.write_fds))
          FD_SET(fd, &fds_in_.write_fds);
      }
      else {
        if (FD_ISSET(fd, &fds_in_.write_fds))
          FD_CLR(fd, &fds_in_.write_fds);
      }
    }
  }
}

void EventSelect::remove_channel(Channel* channel) {
  assert_in_loopthread();

  const int fd = channel->get_fd();
  const int index = channel->get_index();
  CHAOSLOG_TRACE << "EventSelect::remove_channel - fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->is_none_event());
  assert(index == Poller::EVENT_ADD || index == Poller::EVENT_DEL);
  CHAOS_UNUSED(index);

  fds_in_.remove(fd);
  fds_out_.remove(fd);

  if (fd == max_fd_) {
    max_fd_ = 0;
    std::for_each(channels_.begin(), channels_.end(),
        [this](const std::pair<int, Channel*>& channel_pair) {
          if (channel_pair.first > max_fd_)
            max_fd_ = channel_pair.first;
        });
  }
}

void EventSelect::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  for (auto& channel_pair : channels_) {
    if (nevents <= 0)
      break;

    Channel* channel = channel_pair.second;
    const int fd = channel_pair.first;
    int revents = 0;
    if (FD_ISSET(fd, &fds_out_.read_fds))
      revents |= POLLIN;
    if (FD_ISSET(fd, &fds_out_.write_fds))
      revents |= POLLOUT;
    if (FD_ISSET(fd, &fds_out_.error_fds))
      revents |= POLLERR;

    if (revents == 0)
      continue;

    --nevents;
    channel->set_revents(revents);
    active_channels.push_back(channel);
  }
}

}
