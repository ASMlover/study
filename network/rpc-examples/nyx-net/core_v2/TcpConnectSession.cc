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
#include "TcpConnectSession.h"

namespace nyx {

TcpConnectSession::TcpConnectSession(asio::io_context& context)
  : TcpSession(context)
  , resolver_(context) {
}

TcpConnectSession::~TcpConnectSession(void) {
}

void TcpConnectSession::async_connect(
    const std::string& host, std::uint16_t port) {
  host_ = host;
  port_ = port;

  auto self(shared_from_this());
  resolver_.async_resolve(
      asio::ip::tcp::resolver::query(host_, std::to_string(port_)),
      [this, self](const std::error_code& ec, tcp::resolver::iterator epiter) {
        if (!ec) {
          auto ep = *epiter;
          ++epiter;
          socket_.async_connect(*epiter,
              [this, self, epiter](const std::error_code& ec) {
                handle_async_connect(ec, epiter);
              });
        }
        else {
          // TODO: on_resolve_error(...)
          if (stop_impl())
            cleanup();
        }
      });
}

void TcpConnectSession::async_write(const std::string& buf) {
  async_write_impl(buf);
}

void TcpConnectSession::set_option(void) {
  if (!socket_.is_open())
    socket_.open(asio::ip::tcp::v4());

  socket_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  socket_.set_option(asio::ip::tcp::socket::keep_alive(true));
  socket_.set_option(asio::ip::tcp::no_delay(true));
}

void TcpConnectSession::start_impl(void) {
  // TODO: need notify connected
  TcpSession::start_impl();
}

bool TcpConnectSession::stop_impl(void) {
  return TcpSession::stop_impl();
}

void TcpConnectSession::cleanup(void) {
  if (is_connected_)
    is_connected_ = false;
  TcpSession::cleanup();
}

void TcpConnectSession::handle_async_connect(
    const std::error_code& ec, tcp::resolver::iterator epiter) {
  if (!ec) {
    set_option();
    // TODO: notify connected, on_connected(...)

    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(buffer_),
        [this, self](const std::error_code& ec, std::size_t n) {
          handle_async_read(ec, n);
        });
  }
  else if (epiter != tcp::resolver::iterator()) {
    socket_.close();

    auto self(shared_from_this());
    auto ep = *epiter;
    ++epiter;
    socket_.async_connect(ep,
        [this, self, epiter](const std::error_code& ec) {
          handle_async_connect(ec, epiter);
        });
  }
  else {
    // TODO: notify connect error
    if (stop_impl())
      cleanup();
  }
}

void TcpConnectSession::handle_async_read(
    const std::error_code& ec, std::size_t n) {
  if (!ec) {
    // TODO: handle data

    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(buffer_),
        [this, self](const std::error_code& ec, std::size_t n) {
          handle_async_read(ec, n);
        });
  }
  else {
    if (stop_impl())
      cleanup();
  }
}

}
