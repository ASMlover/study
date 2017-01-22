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
#include <cstdio>
#include <functional>
#include <Chaos/Logging/Logging.h>
#include <Chaos/Utility/WeakCallback.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/Channel.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Socket.h>
#include <Neptune/TcpConnection.h>

namespace Neptune {

static const std::size_t kHighWatermark = 64 * 1024 * 1024;

void on_connection_default(const TcpConnectionPtr& conn) {
  CHAOSLOG_TRACE << "on_connection_default - "
    << conn->get_local_address().get_host_port() << " -> "
    << conn->get_peer_address().get_host_port() << " is "
    << (conn->is_connected() ? "ON" : "OFF");
}

void on_message_default(const TcpConnectionPtr& /*conn*/, Buffer* buff, Chaos::Timestamp /*recvtime*/) {
  buff->retrieve_all();
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
    int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr)
  : loop_(CHAOS_CHECK_NONIL(loop))
  , name_(name)
  , linkstate_(NetLink::NETLINK_CONNECTING)
  , socket_(new Socket(sockfd))
  , channel_(new Channel(loop_, sockfd))
  , local_addr_(local_addr)
  , peer_addr_(peer_addr)
  , high_watermark_(kHighWatermark) {
  channel_->bind_read_functor(std::bind(&TcpConnection::do_handle_read, this, std::placeholders::_1));
  channel_->bind_write_functor(std::bind(&TcpConnection::do_handle_write, this));
  channel_->bind_close_functor(std::bind(&TcpConnection::do_handle_close, this));
  channel_->bind_error_functor(std::bind(&TcpConnection::do_handle_error, this));

  CHAOSLOG_DEBUG << "TcpConnection::TcpConnection [" << name_ << "] at " << this << "sockfd=" << sockfd;
  socket_->set_keep_alive(true);
}

TcpConnection::~TcpConnection(void) {
  CHAOSLOG_DEBUG << "TcpConnection::~TcpConnection [" << name_ << "] at "
    << this << " fd=" << channel_->get_fd() << " linkstate=" << linkstate_to_string();
  CHAOS_CHECK(linkstate_ == NetLink::NETLINK_DISCONNECTED, "link state should int disconnected");
}

void TcpConnection::do_connect_established(void) {
  CHAOS_CHECK(linkstate_ == NetLink::NETLINK_CONNECTING, "link state should in connecting");
  loop_->assert_in_loopthread();

  set_linkstate(NetLink::NETLINK_CONNECTED);
  channel_->tie(shared_from_this());
  channel_->enabled_reading();

  connection_fn_(shared_from_this());
}

void TcpConnection::do_connect_destroyed(void) {
  loop_->assert_in_loopthread();

  if (linkstate_ == NetLink::NETLINK_CONNECTED) {
    set_linkstate(NetLink::NETLINK_DISCONNECTED);
    channel_->disabled_all();

    connection_fn_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::write(const Chaos::StringPiece& message) {
  if (linkstate_ == NetLink::NETLINK_CONNECTED) {
    if (loop_->in_loopthread()) {
      write_in_loop(message);
    }
    else {
      void (TcpConnection::*fnp)(const Chaos::StringPiece&) = &TcpConnection::write_in_loop;
      loop_->run_in_loop(std::bind(fnp, this, message));
    }
  }
}

void TcpConnection::write(Buffer* buf) {
  if (linkstate_ == NetLink::NETLINK_CONNECTED) {
    if (loop_->in_loopthread()) {
      write_in_loop(buf->peek(), buf->readable_bytes());
      buf->retrieve_all();
    }
    else {
      void (TcpConnection::*fnp)(const Chaos::StringPiece&) = &TcpConnection::write_in_loop;
      loop_->run_in_loop(std::bind(fnp, this, buf->retrieve_all_to_string()));
    }
  }
}

void TcpConnection::write(void* buf, std::size_t len) {
  write(Chaos::StringPiece(buf, len));
}

void TcpConnection::shutdown(void) {
  if (linkstate_ == NetLink::NETLINK_CONNECTED) {
    set_linkstate(NetLink::NETLINK_DISCONNECTING);
    loop_->run_in_loop([this](void) {
          loop_->assert_in_loopthread();
          if (!channel_->is_writing())
            socket_->shutdown_write();
        });
  }
}

void TcpConnection::force_close(void) {
  if (linkstate_ == NetLink::NETLINK_CONNECTED || linkstate_ == NetLink::NETLINK_DISCONNECTING) {
    set_linkstate(NetLink::NETLINK_DISCONNECTING);

    auto self(shared_from_this());
    loop_->put_in_loop([this, self](void) {
          loop_->assert_in_loopthread();
          if (linkstate_ == NetLink::NETLINK_CONNECTED || linkstate_ == NetLink::NETLINK_DISCONNECTED)
            do_handle_close();
        });
  }
}

void TcpConnection::force_close_with_delay(double seconds) {
  if (linkstate_ == NetLink::NETLINK_CONNECTED || linkstate_ == NetLink::NETLINK_DISCONNECTING) {
    set_linkstate(NetLink::NETLINK_DISCONNECTING);
    loop_->run_after(seconds, Chaos::make_weak_callback(shared_from_this(), &TcpConnection::force_close));
  }
}

void TcpConnection::set_tcp_nodelay(bool nodelay) {
  socket_->set_tcp_nodelay(nodelay);
}

void TcpConnection::start_read(void) {
  loop_->run_in_loop([this](void) {
        loop_->assert_in_loopthread();
        if (!reading_ || !channel_->is_reading()) {
          channel_->enabled_reading();
          reading_ = true;
        }
      });
}

void TcpConnection::stop_read(void) {
  loop_->run_in_loop([this](void) {
        loop_->assert_in_loopthread();
        if (reading_ || channel_->is_reading()) {
          channel_->disabled_reading();
          reading_ = false;
        }
      });
}

void TcpConnection::do_handle_read(Chaos::Timestamp recvtime) {
  loop_->assert_in_loopthread();

  int saved_errno = 0;
  ssize_t len = readbuff_.read_sockfd(channel_->get_fd(), saved_errno);
  if (len > 0) {
    message_fn_(shared_from_this(), &readbuff_, recvtime);
  }
  else if (len == 0) {
    do_handle_close();
  }
  else {
    errno = saved_errno;
    CHAOSLOG_SYSERR << "TcpConnection::do_handle_read - errno=" << saved_errno;
    do_handle_error();
  }
}

void TcpConnection::do_handle_write(void) {
  loop_->assert_in_loopthread();

  if (channel_->is_writing()) {
    ssize_t n = NetOps::socket::write(channel_->get_fd(), writbuff_.peek(), writbuff_.readable_bytes());
    if (n > 0) {
      writbuff_.retrieve(n);
      if (writbuff_.readable_bytes() == 0) {
        channel_->disabled_reading();
        if (write_complete_fn_)
          loop_->put_in_loop(std::bind(write_complete_fn_, shared_from_this()));
        if (linkstate_ == NetLink::NETLINK_DISCONNECTING && !channel_->is_writing())
          socket_->shutdown_write();
      }
      else {
        CHAOSLOG_TRACE << "TcpConnection::do_handle_write - I'm going to write more data";
      }
    }
    else {
      CHAOSLOG_SYSERR << "TcpConnection::do_handle_write";
    }
  }
  else {
    CHAOSLOG_SYSERR << "TcpConnection::do_handle_write - fd="
      << channel_->get_fd() << " is down, no more data for writing";
  }
}

void TcpConnection::do_handle_close(void) {
  loop_->assert_in_loopthread();

  CHAOSLOG_TRACE << "TcpConnection::do_handle_close - fd=" << channel_->get_fd()
    << " linkstate=" << linkstate_to_string();
  CHAOS_CHECK(linkstate_ == NetLink::NETLINK_CONNECTED || linkstate_ == NetLink::NETLINK_DISCONNECTING,
      "link state should in conncted or disconnecting");
  set_linkstate(NetLink::NETLINK_DISCONNECTED);
  channel_->disabled_all();

  auto self(shared_from_this());
  connection_fn_(self);
  close_fn_(self);
}

void TcpConnection::do_handle_error(void) {
  int saved_errno = NetOps::socket::get_errno(channel_->get_fd());
  CHAOSLOG_ERROR << "TcpConnection::do_handle_error - [" << name_
    << "] - SO_ERROR=" << saved_errno << " " << Chaos::strerror_tl(saved_errno);
}

void TcpConnection::write_in_loop(const Chaos::StringPiece& message) {
  write_in_loop(message.data(), message.size());
}

void TcpConnection::write_in_loop(const void* buf, std::size_t len) {
  loop_->assert_in_loopthread();

  ssize_t nwrote = 0;
  std::size_t nremain = len;
  bool fault_error = false;
  if (linkstate_ == NetLink::NETLINK_DISCONNECTED) {
    CHAOSLOG_WARN << "TcpConnection::write_in_loop - disconnected, give up writting";
    return;
  }

  if (!channel_->is_writing() && writbuff_.readable_bytes() == 0) {
    nwrote = NetOps::socket::write(channel_->get_fd(), buf, len);
    if (nwrote >= 0) {
      nremain = len - nwrote;
      if (nremain == 0 && write_complete_fn_)
        loop_->put_in_loop(std::bind(write_complete_fn_, shared_from_this()));
    }
    else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        CHAOSLOG_SYSERR << "TcpConnection::write_in_loop - write errno=" << errno;
        if (errno == EPIPE || errno == ECONNRESET)
          fault_error = true;
      }
    }
  }

  CHAOS_CHECK(nremain <= len, "remain writing data should less equal to `len`");
  if (!fault_error && nremain > 0) {
    std::size_t nold = writbuff_.readable_bytes();
    if (nold + nremain >= high_watermark_ && nold < high_watermark_ && high_watermark_fn_)
      loop_->put_in_loop(std::bind(high_watermark_fn_, shared_from_this(), nold + nremain));
    writbuff_.append(static_cast<const char*>(buf) + nwrote, nremain);
    if (!channel_->is_writing())
      channel_->enabled_writing();
  }
}

const char* TcpConnection::linkstate_to_string(void) const {
  switch (linkstate_) {
  case NetLink::NETLINK_CONNECTING:
    return "NETLINK_CONNECTING";
  case NetLink::NETLINK_CONNECTED:
    return "NETLINK_CONNECTED";
  case NetLink::NETLINK_DISCONNECTING:
    return "NETLINK_DISCONNECTING";
  case NetLink::NETLINK_DISCONNECTED:
    return "NETLINK_DISCONNECTED";
  }
  return "Unknown linkstate";
}

}
