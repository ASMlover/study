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
#ifndef __TYR_NET_ACCEPTOR_HEADER_H__
#define __TYR_NET_ACCEPTOR_HEADER_H__

#include <functional>
#include "../basic/TUnCopyable.h"
#include "TChannel.h"
#include "TSocket.h"

namespace tyr { namespace net {

class EventLoop;
class InetAddress;

class Acceptor : private basic::UnCopyable {
  using NewConnectionCallback = std::function<void (int sockfd, const InetAddress& addr)>;

  EventLoop* loop_{};
  Socket accept_sock_;
  Channel accept_channel_;
  NewConnectionCallback new_connection_fn_{};
  bool listenning_{};

  void handle_read(void);
public:
  Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port);
  ~Acceptor(void);

  void listen(void);

  void set_new_connection_callback(const NewConnectionCallback& fn) {
    new_connection_fn_ = fn;
  }

  bool is_listenning(void) const {
    return listenning_;
  }
};

}}

#endif // __TYR_NET_ACCEPTOR_HEADER_H__
