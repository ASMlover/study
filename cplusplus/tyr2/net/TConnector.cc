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
#include "../basic/TTypes.h"
#include "../basic/TLogging.h"
#include "TCallbacks.h"
#include "TChannel.h"
#include "TEventLoop.h"
#include "TSocketSupport.h"
#include "TConnector.h"

namespace tyr { namespace net {

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
  : loop_(loop)
  , retry_delay_ms_(kInitRetryDelayMillisecond)
  , server_addr_(server_addr) {
  TYRLOG_DEBUG << "Connector::Connector - [" << this << "]";
}

Connector::~Connector(void) {
  TYRLOG_DEBUG << "Connector::~Connector - [" << this << "]";
  assert(!channel_);
}

void Connector::start(void) {
  connect_ = true;
  loop_->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

void Connector::restart(void) {
  loop_->assert_in_loopthread();
  set_state(STATE_DISCONNECTED);
  retry_delay_ms_ = kInitRetryDelayMillisecond;
  connect_ = true;
  start_in_loop();
}

void Connector::stop(void) {
  connect_ = false;
  loop_->put_in_loop(std::bind(&Connector::stop_in_loop, this));
}

void Connector::start_in_loop(void) {
  loop_->assert_in_loopthread();
  assert(state_ == STATE_DISCONNECTED);
  if (connect_)
    connect();
  else
    TYRLOG_DEBUG << "Connector::start_in_loop - do not connect";
}

void Connector::stop_in_loop(void) {
  loop_->assert_in_loopthread();
  if (state_ == STATE_CONNECTING) {
    set_state(STATE_DISCONNECTED);
    int sockfd = remove_and_reset_channel();
    retry(sockfd);
  }
}

void Connector::connect(void) {
  int sockfd = SocketSupport::kern_socket(server_addr_.get_family());
  int ret = SocketSupport::kern_connect(sockfd, server_addr_.get_address());
  int saved_errno = (ret == 0) ? 0 : errno;
  switch (saved_errno) {
  case 0:
  case EINPROGRESS:
  case EINTR:
  case EISCONN:
    connecting(sockfd);
    break;
  case EAGAIN:
  case EADDRINUSE:
  case EADDRNOTAVAIL:
  case ECONNREFUSED:
  case ENETUNREACH:
    retry(sockfd);
    break;
  case EACCES:
  case EPERM:
  case EBADF:
  case EFAULT:
  case EAFNOSUPPORT:
  case EALREADY:
  case ENOTSOCK:
    TYRLOG_SYSERR << "Connector::connect - connect error in Connector::start_in_loop " << saved_errno;
    SocketSupport::kern_close(sockfd);
    break;
  default:
    TYRLOG_SYSERR << "Connector::connect - unexcepted error in Connector::start_in_loop " << saved_errno;
    SocketSupport::kern_close(sockfd);
    break;
  }
}

void Connector::connecting(int sockfd) {
  set_state(STATE_CONNECTING);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->set_write_callback(std::bind(&Connector::handle_write, this));
  channel_->set_error_callback(std::bind(&Connector::handle_error, this));
  channel_->enabled_writing();
}

void Connector::retry(int sockfd) {
  SocketSupport::kern_close(sockfd);
  set_state(STATE_DISCONNECTED);
  if (connect_) {
    TYRLOG_INFO << "Connector::retry - retry connecting to "
      << server_addr_.to_host_port() << " in "
      << retry_delay_ms_ << " milliseconds.";
    loop_->run_after(retry_delay_ms_ / 1000.0, std::bind(&Connector::start_in_loop, shared_from_this()));
    retry_delay_ms_ = basic::tyr_min(retry_delay_ms_ * 2, kMaxRetryDelayMillisecond);
  }
  else {
    TYRLOG_DEBUG << "Connector::retry - do not connect";
  }
}

int Connector::remove_and_reset_channel(void) {
  channel_->disabled_all();
  channel_->remove();
  int sockfd = channel_->get_fd();
  loop_->put_in_loop(std::bind(&Connector::reset_channel, this));
  return sockfd;
}

void Connector::reset_channel(void) {
  channel_.reset();
}

void Connector::handle_write(void) {

  TYRLOG_TRACE << "Connector::handle_write - " << state_;
  if (state_ == STATE_CONNECTING) {
    int sockfd = remove_and_reset_channel();
    int err = SocketSupport::kern_socket_error(sockfd);
    if (err != 0) {
      TYRLOG_WARN << "Connector::handle_write - SO_ERROR=" << err << " " << basic::strerror_tl(err);
      retry(sockfd);
    }
    else if (SocketSupport::kern_is_self_connect(sockfd)) {
      TYRLOG_WARN << "Connector::handle_write - self connect";
      retry(sockfd);
    }
    else {
      set_state(STATE_CONNECTED);
      if (connect_)
        new_connection_fn_(sockfd);
      else
        SocketSupport::kern_close(sockfd);
    }
  }
  else {
    assert(state_ == STATE_DISCONNECTED);
  }
}

void Connector::handle_error(void) {
  TYRLOG_ERROR << "Connector::handle_error - state=" << state_;
  if (state_ == STATE_CONNECTING) {
    int sockfd = remove_and_reset_channel();
    int err = SocketSupport::kern_socket_error(sockfd);
    TYRLOG_TRACE << "Connector::handle_error - SO_ERROR=" << err << " " << basic::strerror_tl(err);
    retry(sockfd);
  }
}

}}
