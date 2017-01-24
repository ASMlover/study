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
#include <cerrno>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/Callbacks.h>
#include <Neptune/Channel.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Connector.h>

namespace Neptune {

const int Connector::kMaxRetryDelayMillisecond;
const int Connector::kInitRetryDelayMillisecond;

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
  : loop_(loop)
  , retry_delay_millisecond_(kInitRetryDelayMillisecond)
  , server_addr_(server_addr) {
  CHAOSLOG_DEBUG << "Connector::Connector - [" << this << "]";
}

Connector::~Connector(void) {
  CHAOSLOG_DEBUG << "Connector::~Connector - [" << this << "]";
  CHAOS_CHECK(!channel_, "`channel_` should be invalid");
}

void Connector::start(void) {
  need_connect_ = true;
  loop_->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

void Connector::stop(void) {
  need_connect_ = false;
  auto self(shared_from_this());
  loop_->put_in_loop([this, self](void) {
        loop_->assert_in_loopthread();
        if (linkstate_ == NetLink::NETLINK_CONNECTING) {
          set_linkstate(NetLink::NETLINK_DISCONNECTED);
          int sockfd = remove_and_reset_channel();
          retry(sockfd);
        }
      });
}

void Connector::restart(void) {
  loop_->assert_in_loopthread();

  set_linkstate(NetLink::NETLINK_DISCONNECTED);
  retry_delay_millisecond_ = kInitRetryDelayMillisecond;
  need_connect_ = true;
  start_in_loop();
}

void Connector::start_in_loop(void) {
  loop_->assert_in_loopthread();

  CHAOS_CHECK(linkstate_ == NetLink::NETLINK_DISCONNECTED, "link state should in disconnected");
  if (need_connect_)
    do_connect();
  else
    CHAOSLOG_DEBUG << "Connector::start_in_loop - do not connect";
}

void Connector::do_connect(void) {
  int sockfd = NetOps::socket::open(server_addr_.get_family());
  int r = NetOps::socket::connect(sockfd, server_addr_.get_address());
  int saved_errno = (r == 0) ? 0 : errno;
  switch (saved_errno) {
  case 0:
  case EINPROGRESS:
  case EINTR:
  case EISCONN:
    do_connecting(sockfd);
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
    CHAOSLOG_SYSERR << "Connector::do_connect - connect failed, errno=" << saved_errno;
    NetOps::socket::close(sockfd);
    break;
  default:
    CHAOSLOG_SYSERR << "Connector::do_connect - unexcepted error, errno=" << saved_errno;
    NetOps::socket::close(sockfd);
    break;
  }
}

void Connector::do_connecting(int sockfd) {
  set_linkstate(NetLink::NETLINK_CONNECTING);
  CHAOS_CHECK(!channel_, "connector channel should invalid");
  channel_.reset(new Channel(loop_, sockfd));
  channel_->bind_write_functor(std::bind(&Connector::do_handle_write, this));
  channel_->bind_error_functor(std::bind(&Connector::do_handle_error, this));
  channel_->enabled_writing();
}

void Connector::do_handle_write(void) {
  CHAOSLOG_TRACE << "Connector::do_handle_write - linkstate=" << linkstate_to_string();
  if (linkstate_ == NetLink::NETLINK_CONNECTING) {
    int sockfd = remove_and_reset_channel();
    int err = NetOps::socket::get_errno(sockfd);
    if (err != 0) {
      CHAOSLOG_WARN << "Connector::do_handle_write - SO_ERROR=" << err << " " << Chaos::strerror_tl(err);
      retry(sockfd);
    }
    else if (NetOps::socket::is_self_connect(sockfd)) {
      CHAOSLOG_WARN << "Connector::do_handle_write - in self connect";
      retry(sockfd);
    }
    else {
      set_linkstate(NetLink::NETLINK_CONNECTED);
      if (need_connect_)
        new_connection_fn_(sockfd);
      else
        NetOps::socket::close(sockfd);
    }
  }
  else {
    CHAOS_CHECK(linkstate_ == NetLink::NETLINK_DISCONNECTED, "link state is disconnected");
  }
}

void Connector::do_handle_error(void) {
  CHAOSLOG_ERROR << "Connector::do_handle_error - linkstate=" << linkstate_to_string();
  if (linkstate_ == NetLink::NETLINK_CONNECTING) {
    int sockfd = remove_and_reset_channel();
    int err = NetOps::socket::get_errno(sockfd);
    CHAOSLOG_TRACE << "Connector::do_handle_error - SO_ERROR=" << err << " " << Chaos::strerror_tl(err);
    retry(sockfd);
  }
}

void Connector::retry(int sockfd) {
  NetOps::socket::close(sockfd);
  set_linkstate(NetLink::NETLINK_DISCONNECTED);
  if (need_connect_) {
    CHAOSLOG_INFO << "Connector::retry - retry connecting to "
      << server_addr_.get_host_port() << " in "
      << retry_delay_millisecond_ << " milliseconds.";
    loop_->run_after(retry_delay_millisecond_ / 1000.0, std::bind(&Connector::start_in_loop, shared_from_this()));
    retry_delay_millisecond_ = Chaos::chaos_min(retry_delay_millisecond_ * 2, kMaxRetryDelayMillisecond);
  }
  else {
    CHAOSLOG_DEBUG << "Connector::retry - do not connect";
  }
}

int Connector::remove_and_reset_channel(void) {
  int sockfd = channel_->get_fd();
  channel_->disabled_all();
  channel_->remove();

  auto self(shared_from_this());
  loop_->put_in_loop([this, self] {
        channel_.reset();
      });
  return sockfd;
}

const char* Connector::linkstate_to_string(void) const {
  switch (linkstate_) {
  case NetLink::NETLINK_DISCONNECTED:
    return "NETLINK_DISCONNECTED";
  case NetLink::NETLINK_CONNECTING:
    return "NETLINK_CONNECTING";
  case NetLink::NETLINK_CONNECTED:
    return "NETLINK_CONNECTED";
  }
  return "Unknown linkstate";
}

}
