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
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Channel.h>
#include <Neptune/Windows/EventSelect.h>

namespace Neptune {

EventSelect::FdsEntity::FdsEntity(int fdcount) {
  CHAOS_UNUSED(fdcount);
  // TODO:
}

EventSelect::FdsEntity::~FdsEntity(void) {
  // TODO:
}

void EventSelect::FdsEntity::destroy(void) {
  // TODO:
}

bool EventSelect::FdsEntity::resize(int new_fdcount) {
  // TODO:
  CHAOS_UNUSED(new_fdcount);
  return false;
}

void EventSelect::FdsEntity::copy(const FdsEntity& r) {
  // TODO:
  CHAOS_UNUSED(r);
}

void EventSelect::FdsEntity::remove(int fd) {
  // TODO:
  CHAOS_UNUSED(fd);
}

EventSelect::EventSelect(EventLoop* loop)
  : Poller(loop)
  , fds_in_(FD_SETSIZE)
  , fds_out_(FD_SETSIZE) {
  // TODO:
}

EventSelect::~EventSelect(void) {
  // TODO:
}

Chaos::Timestamp EventSelect::poll(int timeout, std::vector<Channel*>& active_channels) {
  // TODO:
  CHAOS_UNUSED(timeout), CHAOS_UNUSED(active_channels);
}

void EventSelect::update_channel(Channel* channel) {
  // TODO:
  CHAOS_UNUSED(channel);
}

void EventSelect::remove_channel(Channel* channel) {
  // TODO:
  CHAOS_UNUSED(channel);
}

void EventSelect::fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const {
  // TODO:
  CHAOS_UNUSED(nevents), CHAOS_UNUSED(active_channels);
}

}
