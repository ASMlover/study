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
#include "ServerManager.h"
#include "TcpListenSession.h"
#include "TcpServer.h"

namespace nyx {

TcpServer::TcpServer(void)
  : BaseServer(ServerManager::get_instance().get_context())
  , acceptor_(context_) {
}

TcpServer::~TcpServer(void) {
}

void TcpServer::start_impl(void) {
  BaseServer::start_impl();

  auto self(shared_from_this());
  context_.post([this, self] {
        acceptor_.listen(backlog_);
        acceptor_.async_accept(new_conn_->get_socket(),
            [this, self](const std::error_code& ec) {
              handle_async_accept(ec);
            });
      });
}

void TcpServer::stop_impl(void) {
  BaseServer::stop_impl();
  acceptor_.close();
}

void TcpServer::bind(const std::string& host, std::uint16_t port) {
  if (get_status() != Status::INIT)
    return;

  if (host_ == host && port_ == port)
    return;
  host_ = host;
  port_ = port;

  tcp::resolver resolver(context_);
  tcp::resolver::query query(host_, std::to_string(port_));
  tcp::endpoint ep = *resolver.resolve(query);

  if (acceptor_.is_open())
    acceptor_.close();
  acceptor_.open(ep.protocol());
  acceptor_.set_option(tcp::acceptor::reuse_address(reuse_addr_));
  acceptor_.set_option(tcp::socket::keep_alive(true));
  acceptor_.set_option(tcp::no_delay(true));
  acceptor_.bind(ep);
}

void TcpServer::listen(int backlog) {
  backlog_ = backlog;
  reset_connection();
  start_impl();
}

void TcpServer::reset_connection(void) {
  new_conn_.reset(new TcpListenSession(context_));
  new_conn_->set_local_endpoint(host_, port_);
  new_conn_->register_session();
}

void TcpServer::handle_async_accept(const std::error_code& ec) {
  if (!ec) {
    new_conn_->set_callback_handler(get_shared_callback());
    new_conn_->start();
    reset_connection();

    auto self(shared_from_this());
    acceptor_.async_accept(new_conn_->get_socket(),
        [this, self](const std::error_code& ec) {
          handle_async_accept(ec);
        });
  }
}

}
