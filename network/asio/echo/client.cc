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
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcp_client : private boost::noncopyable, public std::enable_shared_from_this<tcp_client> {
  tcp::socket socket_;
  tcp::resolver::iterator epiter_;

  void do_connect(void) {
    boost::asio::async_connect(socket_, epiter_,
        [this](boost::system::error_code ec, tcp::resolver::iterator) {
          if (!ec) {
            do_write();
            do_read();
          }
        });
  }

  void do_read(void) {
    char buf[1024]{};
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buf, sizeof(buf)),
        [this, self, &buf](boost::system::error_code ec, std::size_t /*n*/) {
          if (!ec)
            std::cout << "echo reply: " << buf << std::endl;

          socket_.close();
        });
  }

  void do_write(void) {
    std::string echo_msg = "Hello, world!";
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(echo_msg),
        [self](boost::system::error_code /*ec*/, std::size_t /*n*/) {
        });
  }
public:
  tcp_client(boost::asio::io_service& io_service, tcp::resolver::iterator epiter)
    : socket_(io_service)
    , epiter_(epiter) {
  }

  void start(void) {
    do_connect();
  }
};

int main(int argc, char* argv[]) {
  ((void)argc), ((void)argv);

  boost::asio::io_service io_service;
  tcp::socket socket(io_service);
  tcp::resolver r(io_service);
  boost::asio::async_connect(socket, r.resolve({"127.0.0.1", "5555"}),
      [&socket](boost::system::error_code ec, tcp::resolver::iterator) {
        if (!ec) {
          std::string echo = "Hello, world!";
          boost::asio::write(socket, boost::asio::buffer(echo));

          char buf[1024]{};
          socket.async_read_some(boost::asio::buffer(buf, sizeof(buf)),
              [&socket, &buf](boost::system::error_code ec, std::size_t) {
                if (!ec)
                  std::cout << "echo client recv: " << buf << std::endl;
                socket.close();
              });
        }
      });

  io_service.run();

  return 0;
}
