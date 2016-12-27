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
#include "../basic/TWeakCallback.h"
#include "TSocketSupport.h"
#include "TChannel.h"
#include "TEventLoop.h"
#include "TSocket.h"
#include "TTcpConnection.h"

#define TYR_HIGH_WATERMARK (64 * 1024 * 1024)

namespace tyr { namespace net {

void default_connection_callback(const TcpConnectionPtr& conn) {
  TYRLOG_TRACE << "default_connection_callback - "
    << conn->get_local_address().to_host_port() << " -> "
    << conn->get_peer_address().to_host_port() << " is "
    << (conn->is_connected() ? "UP" : "DOWN");
}

void default_message_callback(const TcpConnectionPtr& /*conn*/, Buffer* buf, basic::Timestamp /*t*/) {
  buf->retrieve_all();
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
      int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr)
  : loop_(TCHECK_NOTNULL(loop))
  , name_(name)
  , state_(STATE_CONNECTING)
  , socket_(new Socket(sockfd))
  , channel_(new Channel(loop, sockfd))
  , local_addr_(local_addr)
  , peer_addr_(peer_addr)
  , high_water_mark_(TYR_HIGH_WATERMARK) {
  channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
  channel_->set_write_callback(std::bind(&TcpConnection::handle_write, this));
  channel_->set_close_callback(std::bind(&TcpConnection::handle_close, this));
  channel_->set_error_callback(std::bind(&TcpConnection::handle_error, this));

  TYRLOG_DEBUG << "TcpConnection::TcpConnection [" << name_ << "] at " << this << " fd=" << sockfd;
  socket_->set_keep_alive(true);
}

TcpConnection::~TcpConnection(void) {
  TYRLOG_DEBUG << "TcpConnection::~TcpConnection [" << name_ << "] at "
    << this << " fd=" << channel_->get_fd() << " state=" << state_to_string();
  assert(state_ == STATE_DISCONNECTED);
}

void TcpConnection::connect_established(void) {
  loop_->assert_in_loopthread();
  assert(state_ == STATE_CONNECTING);
  set_state(STATE_CONNECTED);
  channel_->tie(shared_from_this());
  channel_->enabled_reading();

  connection_fn_(shared_from_this());
}

void TcpConnection::connect_destroyed(void) {
  loop_->assert_in_loopthread();
  if (state_ == STATE_CONNECTED) {
    set_state(STATE_DISCONNECTED);
    channel_->disabled_all();

    connection_fn_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::write(const basic::StringPiece& message) {
  if (STATE_CONNECTED == state_) {
    if (loop_->in_loopthread()) {
      write_in_loop(message);
    }
    else {
      void (TcpConnection::*fnp)(const basic::StringPiece& message) = &TcpConnection::write_in_loop;
      loop_->run_in_loop(std::bind(fnp, this, message));
    }
  }
}

void TcpConnection::write(Buffer* buf) {
  if (STATE_CONNECTED == state_) {
    if (loop_->in_loopthread()) {
      write_in_loop(buf->peek(), buf->readable_bytes());
      buf->retrieve_all();
    }
    else {
      void (TcpConnection::*fnp)(const basic::StringPiece& message) = &TcpConnection::write_in_loop;
      loop_->run_in_loop(std::bind(fnp, this, buf->retrieve_all_to_string()));
    }
  }
}

void TcpConnection::write(void* buf, size_t len) {
  write(basic::StringPiece(static_cast<const char*>(buf), len));
}

void TcpConnection::shutdown(void) {
  if (STATE_CONNECTED == state_) {
    set_state(STATE_DISCONNECTING);
    loop_->run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, this));
  }
}

void TcpConnection::force_close(void) {
  if (STATE_CONNECTED == state_ || STATE_DISCONNECTING == state_) {
    set_state(STATE_DISCONNECTING);
    loop_->put_in_loop(std::bind(&TcpConnection::force_close_in_loop, shared_from_this()));
  }
}

void TcpConnection::force_close_with_delay(double seconds) {
  if (STATE_CONNECTED == state_ || STATE_DISCONNECTING == state_) {
    set_state(STATE_DISCONNECTING);
    loop_->run_after(seconds, basic::make_weak_callback(shared_from_this(), &TcpConnection::force_close));
  }
}

void TcpConnection::set_tcp_nodelay(bool nodelay) {
  socket_->set_tcp_nodelay(nodelay);
}

void TcpConnection::start_read(void) {
  loop_->run_in_loop(std::bind(&TcpConnection::start_read_in_loop, this));
}

void TcpConnection::stop_read(void) {
  loop_->run_in_loop(std::bind(&TcpConnection::stop_read_in_loop, this));
}

void TcpConnection::handle_read(basic::Timestamp recv_time) {
  loop_->assert_in_loopthread();
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
    TYRLOG_SYSERR << "TcpConnection::handle_read - errno=" << saved_errno;
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
        if (write_complete_fn_)
          loop_->put_in_loop(std::bind(write_complete_fn_, shared_from_this()));
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
    TYRLOG_TRACE << "TcpConnection::handle_write - connection="
      << channel_->get_fd() << " is down, no more data for writing";
  }
}

void TcpConnection::handle_close(void) {
  loop_->assert_in_loopthread();
  TYRLOG_TRACE << "TcpConnection::handle_close - fd=" << channel_->get_fd() << " state=" << state_to_string();
  assert(STATE_CONNECTED == state_ || STATE_DISCONNECTING == state_);
  set_state(STATE_DISCONNECTED);
  channel_->disabled_all();

  TcpConnectionPtr this_conn(shared_from_this());
  connection_fn_(this_conn);
  close_fn_(this_conn);
}

void TcpConnection::handle_error(void) {
  int err = SocketSupport::kern_socket_error(channel_->get_fd());
  TYRLOG_ERROR << "TcpConnection::handle_error [" << name_
    << "] - SO_ERROR=" << err << " " << basic::strerror_tl(err);
}

void TcpConnection::write_in_loop(const basic::StringPiece& message) {
  write_in_loop(message.data(), message.size());
}

void TcpConnection::write_in_loop(const void* buf, size_t len) {
  loop_->assert_in_loopthread();
  ssize_t nwrote = 0;
  size_t remain_len = len;
  bool fault_error = false;

  if (STATE_DISCONNECTED == state_) {
    TYRLOG_WARN << "TcpConnection::write_in_loop - disconnected, give up writing";
    return;
  }

  if (!channel_->is_writing() && 0 == output_buff_.readable_bytes()) {
    nwrote = SocketSupport::kern_write(channel_->get_fd(), buf, len);
    if (nwrote >= 0) {
      remain_len = len - nwrote;
      if (0 == remain_len && write_complete_fn_)
        loop_->put_in_loop(std::bind(write_complete_fn_, shared_from_this()));
    }
    else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        TYRLOG_SYSERR << "TcpConnection::write_in_loop - write error=" << errno;
        if (EPIPE == errno || ECONNRESET == errno)
          fault_error = true;
      }
    }
  }

  assert(remain_len <= len);
  if (!fault_error && remain_len > 0) {
    size_t old_len = output_buff_.readable_bytes();
    if (old_len + remain_len >= high_water_mark_ && old_len < high_water_mark_ && high_water_mark_fn_)
      loop_->put_in_loop(std::bind(high_water_mark_fn_, shared_from_this(), old_len + remain_len));

    output_buff_.append(static_cast<const char*>(buf) + nwrote, remain_len);
    if (!channel_->is_writing())
      channel_->enabled_writing();
  }
}

void TcpConnection::shutdown_in_loop(void) {
  loop_->assert_in_loopthread();
  if (!channel_->is_writing())
    socket_->shutdown_write();
}

void TcpConnection::force_close_in_loop(void) {
  loop_->assert_in_loopthread();
  if (STATE_CONNECTED == state_ || STATE_DISCONNECTING == state_)
    handle_close();
}

const char* TcpConnection::state_to_string(void) const {
  switch (state_) {
  case STATE_DISCONNECTED:
    return "STATE_DISCONNECTED";
  case STATE_CONNECTING:
    return "STATE_CONNECTING";
  case STATE_CONNECTED:
    return "STATE_CONNECTED";
  case STATE_DISCONNECTING:
    return "STATE_DISCONNECTING";
  }
  return "Unknown state";
}

void TcpConnection::start_read_in_loop(void) {
  loop_->assert_in_loopthread();
  if (!reading_ || !channel_->is_reading()) {
    channel_->enabled_reading();
    reading_ = true;
  }
}

void TcpConnection::stop_read_in_loop(void) {
  loop_->assert_in_loopthread();
  if (reading_ || channel_->is_reading()) {
    channel_->disabled_reading();
    reading_ = false;
  }
}

}}
