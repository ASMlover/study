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
#ifndef __TYR_NET_EVENTLOOP_HEADER_H__
#define __TYR_NET_EVENTLOOP_HEADER_H__

#include <memory>
#include <vector>
#include "../basic/TUnCopyable.h"
#include "../basic/TPlatform.h"
#include "../basic/TCurrentThread.h"

namespace tyr { namespace net {

class Channel;
class Poller;

class EventLoop : private basic::UnCopyable {
  bool looping_{};
  bool quit_{};
  const pid_t tid_{};
  std::unique_ptr<Poller> poller_;
  std::vector<Channel*> active_channels_;

  void abort_not_in_loopthread(void);
public:
  EventLoop(void);
  ~EventLoop(void);

  void loop(void);
  void quit(void);
  void update_channel(Channel* channel);

  void assert_in_loopthread(void) {
    if (!in_loopthread())
      abort_not_in_loopthread();
  }

  bool in_loopthread(void) const {
    return tid_ == basic::CurrentThread::tid();
  }
};

}}

#endif // __TYR_NET_EVENTLOOP_HEADER_H__
