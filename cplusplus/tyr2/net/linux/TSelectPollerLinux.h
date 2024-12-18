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
#ifndef __TYR_NET_LINUX_SELECTPOLLERLINUX_HEADER_H__
#define __TYR_NET_LINUX_SELECTPOLLERLINUX_HEADER_H__

#include <sys/select.h>
#include "../TPoller.h"

namespace tyr { namespace net {

class Channel;

class SelectPoller : public Poller {
  struct FdsEntity {
    fd_set esets;
    fd_set rsets;
    fd_set wsets;

    FdsEntity(void);
    ~FdsEntity(void);

    void copy(const FdsEntity& r);
    void remove(int fd);
  };

  // fd_set is a fixed size buffer in Linux, can not regrow.
  int max_fd_{}; // record the highest-numbered filedes in any of three sets, plus 1
  FdsEntity sets_in_;
  FdsEntity sets_out_;

  void fill_active_channels(int nevents, std::vector<Channel*>* active_channels) const;
public:
  explicit SelectPoller(EventLoop* loop);
  virtual ~SelectPoller(void);

  virtual basic::Timestamp poll(int timeout, std::vector<Channel*>* active_channels) override;
  virtual void update_channel(Channel* channel) override;
  virtual void remove_channel(Channel* channel) override;
};

}}

#endif // __TYR_NET_LINUX_SELECTPOLLERLINUX_HEADER_H__
