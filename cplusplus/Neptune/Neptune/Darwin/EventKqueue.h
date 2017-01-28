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
#ifndef NEPTUNE_DARWIN_EVENTKQUEUE_H
#define NEPTUNE_DARWIN_EVENTKQUEUE_H

#include <Neptune/Poller.h>

struct kevent;

namespace Neptune {

class Channel;
class EventLoop;

class EventKqueue : public Poller {
  static const int kInitEventsCount = 16;

  int kqueuefd_{};
  std::vector<struct kevent> kqueue_events_;

  void fill_active_channels(int nevents, std::vector<Channel*>& active_channels) const;
  void update(int operation, Channel* channel);
  int kqueue_event_add(int fd, Channel* channel);
  int kqueue_event_del(int fd);
  int kqueue_event_mod(int fd, Channel* channel);
  int kqueue_event_ctl(int operation, int fd, Channel* channel);
  static const char* operation_to_string(int operation);
public:
  explicit EventKqueue(EventLoop* loop);
  virtual ~EventKqueue(void) override;

  virtual Chaos::Timestamp poll(int timeout, std::vector<Channel*>& active_channels) override;
  virtual void update_channel(Channel* channel) override;
  virtual void remove_channel(Channel* channel) override;
};

}

#endif // NEPTUNE_DARWIN_EVENTKQUEUE_H
