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
#ifndef __TYR_NET_POSIX_ASYNCPOLLERLINUX_HEADER_H__
#define __TYR_NET_POSIX_ASYNCPOLLERLINUX_HEADER_H__

#include <vector>
#include "../TPoller.h"

struct epoll_event;

namespace tyr { namespace net {

class Channel;
class EventLoop;

class AsyncPoller : public Poller {
  static const int kInitNumEvents = 16;

  int epollfd_{};
  std::vector<struct epoll_event> epoll_events_;

  static const char* operation_to_string(int op);
  void update(int operation, Channel* channel);
  void fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const;
public:
  explicit AsyncPoller(EventLoop* loop);
  virtual ~AsyncPoller(void);

  virtual basic::Timestamp poll(int timeout, std::vector<Channel*>* active_channels) override;
  virtual void update_channel(Channel* channel) override;
  virtual void remove_channel(Channel* channel) override;
};

}}

#endif // __TYR_NET_POSIX_ASYNCPOLLERLINUX_HEADER_H__
