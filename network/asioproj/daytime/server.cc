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
#include <ctime>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string get_daytime(void) {
  std::time_t now = std::time(nullptr);
  return std::ctime(&now);
}

class tcp_connection
  : private boost::noncopyable
  , public std::enable_shared_from_this<tcp_connection> {
  tcp::socket socket_;

  void do_write(void) {
    std::string message(get_daytime());
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(message),
        [this, self](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {
          socket_.close();
        });
  }
public:
  tcp_connection(tcp::socket&& socket)
    : socket_(std::move(socket)) {
  }

  ~tcp_connection(void) {
    std::cout << "tcp_connection::~tcp_connection" << std::endl;
  }

  void start(void) {
    do_write();
  }
};

class tcp_server : private boost::noncopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;

  void do_accept(void) {
    acceptor_.async_accept(socket_,
        [this](const boost::system::error_code& ec) {
          if (!ec)
            std::make_shared<tcp_connection>(std::move(socket_))->start();

          do_accept();
        });
  }
public:
  tcp_server(boost::asio::io_service& io_service, short port = 5555)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    , socket_(io_service) {
  }

  void start(void) {
    do_accept();
  }
};

int main(int argc, char* argv[]) {
  (void)argc; (void)argv;

  boost::asio::io_service io_service;

  tcp_server daytime_server(io_service);
  daytime_server.start();

  io_service.run();

  return 0;
}
