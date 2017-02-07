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
#include <boost/lexical_cast.hpp>
#include "tcp_server.h"

TcpServer::TcpServer(boost::asio::io_service& io_service)
  : ServerBase(io_service)
  , acceptor_(io_service_) {
}

TcpServer::~TcpServer(void) {
}

void TcpServer::start(void) {
  ServerBase::start();

  auto self(shared_from_this());
  io_service_.post([this, self] {
        acceptor_.set_option(tcp::socket::keep_alive(true));
        acceptor_.set_option(tcp::no_delay(true));
#if defined(__linux__)
        acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_TCP, TCP_KEEPIDLE>(60));
        acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_TCP, TCP_KEEPINTVL>(60));
        acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_TCP, TCP_KEEPCNT>(3));
#endif

        acceptor_.listen(backlog_);
        do_accept();
      });
}

void TcpServer::stop(void) {
  ServerBase::stop();
  acceptor_.close();
}

void TcpServer::bind(const std::string& addr, std::uint16_t port) {
  if (get_status() != ServerBase::SStatus::SS_INIT)
    return;

  if (addr_ == addr && port_ == port)
    return;

  addr_ = addr;
  port_ = port;

  tcp::resolver r(io_service_);
  tcp::resolver::query query(addr_, boost::lexical_cast<std::string>(port_));
  tcp::endpoint endpoint = *r.resolve(query);

  if (acceptor_.is_open())
    acceptor_.close();

  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(tcp::acceptor::reuse_address(reuse_addr_));
  acceptor_.bind(endpoint);
}

void TcpServer::listen(int backlog) {
  backlog_ = backlog;
  reset_connection();
  TcpServer::start();
}

void TcpServer::reset_connection(void) {
  new_connection_.reset(new TcpConnection(io_service_));
  new_connection_->bind_local_endpoint(addr_, port_);
  new_connection_->do_register();
}

void TcpServer::do_accept(void) {
  auto self(shared_from_this());
  acceptor_.async_accept(new_connection_->get_socket(),
      [this, self](const boost::system::error_code& ec) {
        if (!ec) {
          new_connection_->start();
          reset_connection();

          do_accept();
        }
      });
}
