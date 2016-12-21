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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <functional>
#include "../basic/TLogging.h"
#include "TSocketSupport.h"
#include "TChannel.h"
#include "TEventLoop.h"
#include "TSocket.h"
#include "TTcpConnection.h"

namespace tyr { namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
      int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr)
  : loop_(TCHECK_NOTNULL(loop))
  , name_(name)
  , state_(STATE_CONNECTING)
  , socket_(new Socket(sockfd))
  , channel_(new Channel(loop, sockfd))
  , local_addr_(local_addr)
  , peer_addr_(peer_addr) {
  TYRLOG_DEBUG << "TcpConnection::ctor [" << name_ << "] at " << this << " fd = " << sockfd;
  channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
  channel_->set_write_callback(std::bind(&TcpConnection::handle_write, this));
  channel_->set_close_callback(std::bind(&TcpConnection::handle_close, this));
  channel_->set_error_callback(std::bind(&TcpConnection::handle_error, this));
}

TcpConnection::~TcpConnection(void) {
  TYRLOG_DEBUG << "TcpConnection::dtor [" << name_ << "] at " << this << " fd = " << channel_->get_fd();
}

void TcpConnection::connect_established(void) {
  loop_->assert_in_loopthread();
  assert(state_ == STATE_CONNECTING);
  set_state(STATE_CONNECTED);
  channel_->enabled_reading();
  connection_fn_(shared_from_this());
}

void TcpConnection::connect_destroyed(void) {
  loop_->assert_in_loopthread();
  assert(state_ == STATE_CONNECTED || state_ == STATE_DISCONNECTING);
  set_state(STATE_DISCONNECTED);
  channel_->disabled_all();
  connection_fn_(shared_from_this());

  loop_->remove_channel(get_pointer(channel_));
}

void TcpConnection::write(const std::string& message) {
  if (STATE_CONNECTED == state_) {
    if (loop_->in_loopthread())
      write_in_loop(message);
    else
      loop_->run_in_loop(std::bind(&TcpConnection::write_in_loop, this, message));
  }
}

void TcpConnection::shutdown(void) {
  if (STATE_CONNECTED == state_) {
    set_state(STATE_DISCONNECTING);
    loop_->run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, this));
  }
}

void TcpConnection::set_tcp_nodelay(bool nodelay) {
  socket_->set_tcp_nodelay(nodelay);
}

void TcpConnection::handle_read(basic::Timestamp recv_time) {
  int saved_errno = 0;
  ssize_t len = input_buff_.read_fd(channel_->get_fd(), saved_errno);
  if (len > 0) {
    message_fn_(shared_from_this(), &input_buff_, recv_time);
  }
  else if (0 == len) {
    handle_close();
  }
  else {
    errno = saved_errno;
    TYRLOG_SYSERR << "TcpConnection::handle_read";
    handle_error();
  }
}

void TcpConnection::handle_write(void) {
  loop_->assert_in_loopthread();
  if (channel_->is_writing()) {
    ssize_t n = SocketSupport::kern_write(channel_->get_fd(), output_buff_.peek(), output_buff_.readable_bytes());
    if (n > 0) {
      output_buff_.retrieve(n);
      if (0 == output_buff_.readable_bytes()) {
        channel_->disabled_writing();
        if (STATE_DISCONNECTING == state_)
          shutdown_in_loop();
      }
      else {
        TYRLOG_TRACE << "TcpConnection::handle_write - I'm going to write more data";
      }
    }
    else {
      TYRLOG_SYSERR << "TcpConnection::handle_write";
    }
  }
  else {
    TYRLOG_TRACE << "TcpConnection::handle_write - connection is down, no more data for writing";
  }
}

void TcpConnection::handle_close(void) {
  loop_->assert_in_loopthread();
  TYRLOG_TRACE << "TcpConnection::handle_close state = " << state_;
  assert(STATE_CONNECTED == state_ || STATE_DISCONNECTING == state_);
  channel_->disabled_all();
  close_fn_(shared_from_this());
}

void TcpConnection::handle_error(void) {
  int err = SocketSupport::kern_socket_error(channel_->get_fd());
  TYRLOG_TRACE << "TcpConnection::handle_error [" << name_
    << "] - SO_ERROR = " << err << " " << basic::strerror_tl(err);
}

void TcpConnection::write_in_loop(const std::string& message) {
  loop_->assert_in_loopthread();
  ssize_t nwrote = 0;
  if (!channel_->is_writing() && 0 == output_buff_.readable_bytes()) {
    nwrote = SocketSupport::kern_write(channel_->get_fd(), message.data(), message.size());
    if (nwrote >= 0) {
      if (basic::implicit_cast<size_t>(nwrote) < message.size())
        TYRLOG_TRACE << "TcpConnection::write_in_loop - I'm going to write more data";
    }
    else {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
        TYRLOG_SYSERR << "TcpConnection::write_in_loop";
    }
  }

  assert(nwrote >= 0);
  if (basic::implicit_cast<size_t>(nwrote) < message.size()) {
    output_buff_.append(message.data() + nwrote, message.size() - nwrote);
    if (!channel_->is_writing())
      channel_->enabled_writing();
  }
}

void TcpConnection::shutdown_in_loop(void) {
  loop_->assert_in_loopthread();
  if (!channel_->is_writing())
    socket_->shutdown_write();
}

}}
