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
#include "connection_manager.h"
#include "tcp_connection.h"

TcpConnection::TcpConnection(boost::asio::io_service& io_service)
  : ConnectionBase(io_service)
  , socket_(io_service)
  , buffer_(128) {
}

TcpConnection::~TcpConnection(void) {
}

void TcpConnection::do_start(void) {
  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
        if (!ec)
          do_start();
      });
}

bool TcpConnection::do_stop(void) {
  bool false_value = false;
  if (!closed_.compare_exchange_strong(false_value, true, std::memory_order_release, std::memory_order_relaxed))
    return false;

  auto self(shared_from_this());
  strand_->post([this, self] { socket_.close(); });
  return true;
}

void TcpConnection::do_disconnect(void) {
  if (disconnected_)
    return;

  disconnected_ = true;
  if (!sending_ && do_stop())
    do_unregister();
}

const std::string TcpConnection::get_remote_addr(void) const {
  return socket_.lowest_layer().remote_endpoint().address().to_string();
}

std::int16_t TcpConnection::get_remote_port(void) const {
  return socket_.lowest_layer().remote_endpoint().port();
}

const std::string TcpConnection::get_local_addr(void) const {
  return socket_.lowest_layer().local_endpoint().address().to_string();
}

std::int16_t TcpConnection::get_local_port(void) const {
  return socket_.lowest_layer().local_endpoint().port();
}
