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
  // TODO:
}

void TcpConnection::do_connect_established(void) {
  // TODO:
}

void TcpConnection::do_connect_destroyed(void) {
  // TODO:
}

void TcpConnection::write(const Chaos::StringPiece& message) {
  // TODO:
}

void TcpConnection::write(Buffer* buf) {
  // TODO:
}

void TcpConnection::write(void* buf, std::size_t len) {
  // TODO:
}

void TcpConnection::shutdown(void) {
  // TODO:
}

void TcpConnection::force_close(void) {
  // TODO:
}

void TcpConnection::force_close_with_delay(double seconds) {
  // TODO:
}

void TcpConnection::set_tcp_nodelay(bool nodelay) {
  // TODO:
}

void TcpConnection::start_read(void) {
  // TODO:
}

void TcpConnection::stop_read(void) {
  // TODO:
}

void TcpConnection::do_handle_read(Chaos::Timestamp recvtime) {
  // TODO:
}

void TcpConnection::do_handle_write(void) {
  // TODO:
}

void TcpConnection::do_handle_close(void) {
  // TODO:
}

void TcpConnection::do_handle_error(void) {
  // TODO:
}

void TcpConnection::write_in_loop(const Chaos::StringPiece& message) {
  // TODO:
}

void TcpConnection::write_in_loop(const void* buf, std::size_t len) {
  // TODO:
}

void TcpConnection::shutdown_in_loop(void) {
  // TODO:
}

void TcpConnection::force_close_in_loop(void) {
  // TODO:
}

const char* TcpConnection::linkstate_to_string(void) const {
  // TODO:
  return "";
}

void TcpConnection::start_read_in_loop(void) {
  // TODO:
}

void TcpConnection::stop_read_in_loop(void) {
  // TODO:
}

}
