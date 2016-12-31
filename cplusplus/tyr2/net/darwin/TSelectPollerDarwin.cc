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
#include "../../basic/TTypes.h"
#include "../../basic/TLogging.h"
#include "../TSocketSupport.h"
#include "../TChannel.h"
#include "TSelectPollerDarwin.h"

namespace tyr { namespace net {

static inline size_t calc_sets_bytes(int nfds) {
  return howmany(nfds, NFDBITS) * sizeof(fd_mask);
}

SelectPoller::SelectPoller(EventLoop* loop)
  : Poller(loop) {
  // TODO:
}

SelectPoller::~SelectPoller(void) {
  // TODO:
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

void SelectPoller::sets_init(void) {
  // TODO:
}

void SelectPoller::sets_clear(void) {
  // TODO:
}

void SelectPoller::sets_regrow(void) {
  // TODO:
}

}}
