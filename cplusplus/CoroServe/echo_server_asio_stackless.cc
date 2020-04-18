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
#include <boost/asio/yield.hpp>
#include "msg_helper.hh"
#include "examples.hh"

namespace echo_server_asio_stackless {

using boost::asio::ip::tcp;

class TcpServer final
  : private coro::UnCopyable, public boost::asio::coroutine {
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  coro::net::Status status_{coro::net::Status::INIT_ACK};
  coro::msg::ReadBuf rbuf_;
  coro::msg::WriteBuf wbuf_;

  struct _ref {
    TcpServer* s_{};

    _ref(TcpServer* s) noexcept : s_{s} {}
    void operator()(std::error_code ec, coro::sz_t n = 0) { (*s_)(ec, n); }
  };
public:
  TcpServer(boost::asio::io_context& io_context, coro::u16_t port) noexcept
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    , socket_(io_context)
    , rbuf_(1024) {
  }

  void operator()(std::error_code ec, coro::sz_t n = 0) {
    reenter(this) for (;;) {
      yield acceptor_.async_accept(socket_, _ref(this));

      if (!ec) {
        yield boost::asio::async_write(
            socket_, boost::asio::buffer("*", 1), _ref(this));

        status_ = coro::net::Status::WAIT_MSG;
        wbuf_.clear();
        while (!ec) {
          yield socket_.async_read_some(boost::asio::buffer(rbuf_), _ref(this));

          if (!ec) {
            for (int i = 0; i < n; ++i) {
              switch (status_) {
              case coro::net::Status::INIT_ACK: break;
              case coro::net::Status::WAIT_MSG:
                if (rbuf_[i] == '^')
                  status_ = coro::net::Status::READ_MSG;
                break;
              case coro::net::Status::READ_MSG:
                if (rbuf_[i] == '$')
                  status_ = coro::net::Status::WAIT_MSG;
                else
                  wbuf_.push_back(rbuf_[i] + 1);
                break;
              }
            }

            yield boost::asio::async_write(
                socket_, boost::asio::buffer(wbuf_), _ref(this));
            wbuf_.clear();
          }
        }
      }

      socket_.close();
    }
  }
};

void launch() {
  boost::asio::io_context io_context;
  auto server = std::make_shared<TcpServer>(io_context, 5555);
  (*server)(std::error_code());

  io_context.run();
}

}

CORO_EXAMPLE(EchoServerAsioStackless,
    esasiol, "an easy echo server use boost.asio with stackless coroutine") {
  echo_server_asio_stackless::launch();
}
