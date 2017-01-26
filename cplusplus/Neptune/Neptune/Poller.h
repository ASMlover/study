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
#ifndef NEPTUNE_POLLER_H
#define NEPTUNE_POLLER_H

#include <map>
#include <vector>
#include <Chaos/UnCopyable.h>
#include <Chaos/Datetime/Timestamp.h>

namespace Neptune {

class Channel;
class EventLoop;

class Poller : private Chaos::UnCopyable {
  EventLoop* owner_loop_{};
protected:
  std::map<int, Channel*> channels_;
public:
  enum EventType {
    EVENT_NEW = -1,
    EVENT_ADD = 1,
    EVENT_DEL = 2,
  };
  explicit Poller(EventLoop* loop);
  virtual ~Poller(void);

  virtual Chaos::Timestamp poll(int timeout, std::vector<Channel*>& active_channels) = 0;
  virtual void update_channel(Channel* channel) = 0;
  virtual void remove_channel(Channel* channel) = 0;
  virtual bool has_channel(Channel* channel) const;

  void assert_in_loopthread(void) const;
  static Poller* get_poller(EventLoop* loop);
};

}

#endif // NEPTUNE_POLLER_H
