// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <boost/asio.hpp>
#include "msg_helper.hh"
#include "examples.hh"

namespace echo_server_asio {

using boost::asio::ip::tcp;

class TcpSession final
  : private coro::UnCopyable
  , public std::enable_shared_from_this<TcpSession> {
  tcp::socket socket_;
  coro::net::Status status_{coro::net::Status::INIT_ACK};
  coro::msg::ReadBuf rbuf_;
  coro::msg::WriteBuf wbuf_;

  void do_ack() {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(wbuf_),
      [this, self](const std::error_code& ec, coro::sz_t /*n*/) {
        if (!ec) {
          status_ = coro::net::Status::WAIT_MSG;
          wbuf_.clear();
          do_read();
        }
        else {
          socket_.close();
        }
      });
  }

  void do_read() {
    if (status_ == coro::net::Status::INIT_ACK) {
      do_ack();
      return;
    }

    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(rbuf_),
      [this, self](const std::error_code& ec, coro::sz_t n) {
        if (!ec) {
          auto [st, buf] = coro::msg::handle_message(status_, rbuf_.data(), n);
          status_ = st;
          coro::msg::buf::append(wbuf_, buf);
          do_wirte();
        }
        else {
          socket_.close();
        }
      });
  }

  void do_wirte() {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(wbuf_),
      [this, self](const std::error_code& ec, coro::sz_t /*n*/) {
        if (!ec) {
          wbuf_.clear();
          do_read();
        }
        else {
          socket_.close();
        }
      });
  }
public:
  TcpSession(tcp::socket&& socket) noexcept
    : socket_(std::move(socket))
    , rbuf_(1024) {
  }

  void start() {
    status_ = coro::net::Status::INIT_ACK;
    wbuf_.clear();
    wbuf_.push_back('*');

    do_ack();
  }
};

class TcpServer final : private coro::UnCopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;

  void do_accept() {
    acceptor_.async_accept(socket_,
      [this](const std::error_code& ec) {
        if (!ec)
          std::make_shared<TcpSession>(std::move(socket_))->start();

        do_accept();
      });
  }
public:
  TcpServer(boost::asio::io_context& io_context, coro::u16_t port) noexcept
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    , socket_(io_context) {
  }

  void start() {
    do_accept();
  }
};

void launch() {
  boost::asio::io_context io_context;
  TcpServer server{io_context, 5555};
  server.start();

  io_context.run();
}

}

CORO_EXAMPLE(EchoServerAsio, esasio, "an easy echo server use boost.asio") {
  echo_server_asio::launch();
}
