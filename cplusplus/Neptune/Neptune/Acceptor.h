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
#ifndef NEPTUNE_ACCEPTOR_H
#define NEPTUNE_ACCEPTOR_H

#include <functional>
#include <Chaos/UnCopyable.h>
#include <Neptune/Channel.h>
#include <Neptune/Socket.h>

namespace Neptune {

class EventLoop;
class InetAddress;

class Acceptor : private Chaos::UnCopyable {
  using NewConnectionFunction = std::function<void (int sockfd, const InetAddress& addr)>;

  EventLoop* loop_{};
  Socket accept_socket_;
  Channel accept_channel_;
  NewConnectionFunction new_connection_fn_{};
  bool listening_{};

  void do_handle_read(void);
public:
  Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port);
  ~Acceptor(void);

  void listen(void);

  void bind_new_connection_functor(const NewConnectionFunction& fn) {
    new_connection_fn_ = fn;
  }

  void bind_new_connection_functor(NewConnectionFunction&& fn) {
    new_connection_fn_ = std::move(fn);
  }

  bool is_listening(void) const {
    return listening_;
  }
};

}

#endif // NEPTUNE_ACCEPTOR_H
