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
#if !defined(_WINDOWS_)
# include <WinSock2.h>
#endif
#include <cassert>
#include <cerrno>
#include <cstring>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Channel.h>
#include <Neptune/Windows/EventSelect.h>

namespace Neptune {

struct FdSet_t {
  u_int fd_count;
  int fd_array[1];
};

#define WINFD_FREE(fds) if (!(fds)) { free(fds); (fds) = nullptr; }

static inline void WINFD_CLR(int fd, FdSet_t* fds) {
  FD_CLR(fd, fds);
}

static inline void WINFD_ZERO(FdSet_t* fds) {
  FD_ZERO(fds);
}

static inline bool WINFD_ISSET(int fd, FdSet_t* fds) {
  return FD_ISSET(fd, fds) != 0;
}

static inline void WINFD_SET(int fd, FdSet_t* fds) {
  u_int i;
  for (i = 0; i < fds->fd_count; ++i) {
    if (fd == fds->fd_array[i])
      break;
  }
  if (i == fds->fd_count) {
    fds->fd_array[i] = fd;
    ++fds->fd_count;
  }
}

static inline void WINFD_COPY(FdSet_t* d_fds, FdSet_t* s_fds) {
  d_fds->fd_count = s_fds->fd_count;
  std::memcpy(d_fds->fd_array, s_fds->fd_array, s_fds->fd_count * sizeof(int));
}

EventSelect::FdsEntity::FdsEntity(int fdcount) {
  if (resize(fdcount)) {
    WINFD_ZERO(read_fds);
    WINFD_ZERO(write_fds);
    WINFD_ZERO(error_fds);
  }
}

EventSelect::FdsEntity::~FdsEntity(void) {
  destroy();
}

void EventSelect::FdsEntity::destroy(void) {
  WINFD_FREE(read_fds);
  WINFD_FREE(write_fds);
  WINFD_FREE(error_fds);
}

bool EventSelect::FdsEntity::resize(int new_fdcount) {
  std::size_t bytes = sizeof(FdSet_t) + (new_fdcount - 1) * sizeof(int);

  if (!(read_fds = (FdSet_t*)realloc(read_fds, bytes)))
    return false;
  do {
    if (!(write_fds = (FdSet_t*)realloc(write_fds, bytes)))
      break;
    if (!(error_fds = (FdSet_t*)realloc(error_fds, bytes)))
      break;

    return true;
  } while (false);

  destroy();
  return false;
}

void EventSelect::FdsEntity::copy(const FdsEntity& r) {
  WINFD_COPY(read_fds, r.read_fds);
  WINFD_COPY(write_fds, r.write_fds);
  WINFD_COPY(error_fds, r.error_fds);
}

void EventSelect::FdsEntity::remove(int fd) {
  WINFD_CLR(fd, read_fds);
  WINFD_CLR(fd, write_fds);
  WINFD_CLR(fd, error_fds);
}

EventSelect::EventSelect(EventLoop* loop)
  : Poller(loop)
  , fd_storage_(FD_SETSIZE)
  , fds_in_(fd_storage_)
  , fds_out_(fd_storage_) {
}

EventSelect::~EventSelect(void) {
}

Chaos::Timestamp EventSelect::poll(int timeout, std::vector<Channel*>& active_channels) {
  struct timeval tv{timeout / 1000, timeout % 1000 * 1000};

  fds_out_.copy(fds_in_);
  int nevents = select(0,
      (struct fd_set*)fds_out_.read_fds,
      (struct fd_set*)fds_out_.write_fds,
      (struct fd_set*)fds_out_.error_fds,
      &tv);
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
      if (channels_.size() >= static_cast<std::size_t>(fd_storage_)) {
        int new_fdcount = (fd_storage_ != 0) ? static_cast<int>(1.5 * fd_storage_) : FD_SETSIZE;
        fds_in_.resize(new_fdcount);
        fds_out_.resize(new_fdcount);
        fd_storage_ = new_fdcount;
      }
      channels_[fd] = channel;
    }
    else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    WINFD_SET(fd, fds_in_.error_fds);
    if (channel->is_reading())
      WINFD_SET(fd, fds_in_.read_fds);
    if (channel->is_writing())
      WINFD_SET(fd, fds_in_.write_fds);
    channel->set_index(Poller::EVENT_ADD);
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
        if (!WINFD_ISSET(fd, fds_in_.read_fds))
          WINFD_SET(fd, fds_in_.read_fds);
      }
      else {
        if (WINFD_ISSET(fd, fds_in_.read_fds))
          WINFD_CLR(fd, fds_in_.read_fds);
      }

      if (channel->is_writing()) {
        if (!WINFD_ISSET(fd, fds_in_.write_fds))
          WINFD_SET(fd, fds_in_.write_fds);
      }
      else {
        if (WINFD_ISSET(fd, fds_in_.write_fds))
          WINFD_CLR(fd, fds_in_.write_fds);
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
  std::size_t n = channels_.erase(fd);
  assert(n == 1); CHAOS_UNUSED(n);

  fds_in_.remove(fd);
  fds_out_.remove(fd);
  channel->set_index(Poller::EVENT_NEW);
}

void EventSelect::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  for (auto& channel_pair : channels_) {
    if (nevents <= 0)
      break;

    Channel* channel = channel_pair.second;
    const int fd = channel->get_fd();
    int revents = 0;
    if (WINFD_ISSET(fd, fds_out_.read_fds))
      revents |= POLLIN;
    if (WINFD_ISSET(fd, fds_out_.write_fds))
      revents |= POLLOUT;
    if (WINFD_ISSET(fd, fds_out_.error_fds))
      revents |= POLLERR;

    if (revents == 0)
      continue;

    --nevents;
    channel->set_revents(revents);
    active_channels.push_back(channel);
  }
}

}
