// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "../core/nyx_io_manager.h"
#include "nyx_connection.h"
#include "nyx_tcp_listen_connection.h"
#include "nyx_tcp_server.h"

namespace nyx { namespace net {

tcp_server::tcp_server(void)
  : server(nyx::core::io_manager::instance().get_io_service())
  , acceptor_(io_service_) {
}

tcp_server::~tcp_server(void) {
}

void tcp_server::start(void) {
  server::start();

  auto self(shared_from_this());
  io_service_.post([this, self] { handle_start_server(); });
}

void tcp_server::stop(void) {
  server::stop();
  acceptor_.close();
}

void tcp_server::bind(const std::string& host, std::uint16_t port) {
  if (get_status() != server::INIT)
    return;

  if (host_ == host && port_ == port)
    return;

  host_ = host;
  port_ = port;
  boost::asio::ip::tcp::resolver res(io_service_);
  boost::asio::ip::tcp::resolver::query query(host_,
      boost::lexical_cast<std::string>(port_));
  boost::asio::ip::tcp::endpoint ep = *res.resolve(query);

  if (acceptor_.is_open())
    acceptor_.close();

  acceptor_.open(ep.protocol());
  acceptor_.set_option(
      boost::asio::ip::tcp::acceptor::reuse_address(reuse_addr_));
  acceptor_.bind(ep);
}

void tcp_server::listen(std::size_t backlog) {
  backlog_ = backlog;
  reset_connection();
  start();
}

void tcp_server::handle_start_server(void) {
  acceptor_.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
  acceptor_.set_option(boost::asio::ip::tcp::no_delay(true));
#if defined(__linux__)
  acceptor_.set_option(connection::keep_idle(60));
  acceptor_.set_option(connection::keep_intvl(30));
#endif

  acceptor_.listen(backlog_);
  auto self(shared_from_this());
  acceptor_.async_accept(new_connection_->get_socket(),
      [this, self](const boost::system::error_code& ec) {
        handle_accept(ec);
      });
}

void tcp_server::reset_connection(void) {
  new_connection_.reset(new tcp_listen_connection(io_service_));
  new_connection_->do_register();
}

void tcp_server::handle_accept(const boost::system::error_code& ec) {
  if (!ec) {
    new_connection_->start();
    reset_connection();

    auto self(shared_from_this());
    acceptor_.async_accept(new_connection_->get_socket(),
        [this, self](const boost::system::error_code& ec) {
          handle_accept(ec);
        });
  }
}

}}
