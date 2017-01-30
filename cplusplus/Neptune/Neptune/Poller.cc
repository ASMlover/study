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
#include <Chaos/Platform.h>
#include <Neptune/Channel.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Poller.h>
#include <Neptune/EventPoll.h>
#include <Neptune/EventSelect.h>
#if defined(CHAOS_LINUX)
# include <Neptune/Linux/EventEpoll.h>
#elif defined(CHAOS_DARWIN)
# include <Neptune/Darwin/EventKqueue.h>
#endif

namespace Neptune {

Poller::Poller(EventLoop* loop)
  : owner_loop_(loop) {
}

Poller::~Poller(void) {
}

bool Poller::has_channel(Channel* channel) const {
  assert_in_loopthread();
  auto it = channels_.find(channel->get_fd());
  return it != channels_.end() && it->second == channel;
}

void Poller::assert_in_loopthread(void) const {
  owner_loop_->assert_in_loopthread();
}

Poller* Poller::get_poller(EventLoop* loop) {
#if defined(NEPTUNE_USE_SELECT)
  return new Neptune::EventSelect(loop);
#elif defined(NEPTUNE_USE_POLL)
  return new Neptune::EventPoll(loop);
#else
# if defined(CHAOS_WINDOWS)
  return new Neptune::EventPoll(loop);
# elif defined(CHAOS_LINUX)
  return new Neptune::EventEpoll(loop);
# elif defined(CHAOS_DARWIN)
  return new Neptune::EventKqueue(loop);
# endif
#endif
  return nullptr;
}

}
