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
#include "../TSocketSupport.h"
#include "../TChannel.h"
#include "TSelectPollerWindows.h"

namespace tyr { namespace net {

struct _FdSet_t {
  u_int fd_count;
  int fd_array[1];
};

#define WINFD_CLR   FD_CLR
#define WINFD_ZERO  FD_ZERO
#define WINFD_ISSET FD_ISSET
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
}

SelectPoller::~SelectPoller(void) {
}

basic::Timestamp SelectPoller::poll(int timeout, std::vector<Channel*>* active_channels) {
  // TODO:
  return basic::Timestamp::now();
}

void SelectPoller::update_channel(Channel* channel) {
  // TODO:
}

void SelectPoller::remove_channel(Channel* channel) {
  // TODO:
}

void SelectPoller::fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const {
  // TODO:
}

void SelectPoller::init_fds(void) {
  // TODO:
}

void SelectPoller::clear_fds(void) {
  // TODO:
}

void SelectPoller::regrow_fds(void) {
  // TODO:
}

}}
