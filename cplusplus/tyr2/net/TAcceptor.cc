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
#include "../basic/TLogging.h"
#include "TInetAddress.h"
#include "TSocketSupport.h"
#include "TEventLoop.h"
#include "TAcceptor.h"

namespace tyr { namespace net {

// bool listenning_{};
// EventLoop* loop_{};
// Socket accept_sock_;
// Channel accept_channel_;
// NewConnectionCallback new_connection_fn_;
Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr)
  : loop_(loop)
  , accept_sock_(SocketSupport::kern_socket(AF_INET))
  , accept_channel_(loop_, accept_sock_.get_fd()) {
  accept_sock_.set_reuse_addr(true);
  accept_sock_.bind_address(listen_addr);
  accept_channel_.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

void Acceptor::listen(void) {
  loop_->assert_in_loopthread();
  listenning_ = true;
  accept_sock_.listen();
  accept_channel_.enabled_reading();
}

void Acceptor::handle_read(void) {
  loop_->assert_in_loopthread();
  InetAddress peeraddr;

  int connfd = accept_sock_.accept(&peeraddr);
  if (connfd >= 0) {
    if (new_connection_fn_)
      new_connection_fn_(connfd, peeraddr);
    else
      SocketSupport::kern_close(connfd);
  }
}

}}
