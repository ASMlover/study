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
#include "TTcpConnection.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <functional>
#include "../basic/TLogging.h"
#include "TChannel.h"
#include "TEventLoop.h"
#include "TSocket.h"
#include "TSocketSupport.h"

namespace tyr { namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
      int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr)
  : loop_(TCHECK_NOTNULL(loop))
  , name_(name)
  , state_(ST_CONNECTING)
  , socket_(new Socket(sockfd))
  , channel_(new Channel(loop, sockfd))
  , local_addr_(local_addr)
  , peer_addr_(peer_addr) {
  TYRLOG_DEBUG << "TcpConnection::ctor [" << name_ << "] at " << this << " fd = " << sockfd;
  channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this));
}

TcpConnection::~TcpConnection(void) {
  TYRLOG_DEBUG << "TcpConnection::dtor [" << name_ << "] at " << this << " fd = " << channel_->get_fd();
}

void TcpConnection::connect_established(void) {
  loop_->assert_in_loopthread();
  assert(state_ == ST_CONNECTING);
  set_state(ST_CONNECTD);
  channel_->enabled_reading();

  connection_fn_(shared_from_this());
}

void TcpConnection::handle_read(void) {
  char buf[65535];
  ssize_t len = SocketSupport::kern_read(channel_->get_fd(), buf, sizeof(buf));
  message_fn_(shared_from_this(), buf, len);
}

}}
