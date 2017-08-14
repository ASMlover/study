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
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class Client : private boost::noncopyable {
  udp::socket socket_;

  static constexpr std::size_t kMaxBufferSize = 1024;

  void do_connect(const udp::endpoint& remote_ep) {
    socket_.async_connect(remote_ep,
        [this](const boost::system::error_code& ec) {
          if (!ec) {
            do_read();
            do_write();
          }
          else {
            socket_.close();
          }
        });
  }

  void do_read(void) {
    char readbuff[kMaxBufferSize];
    socket_.async_receive(boost::asio::buffer(readbuff, sizeof(readbuff)),
        [this, &readbuff](const boost::system::error_code& ec, std::size_t) {
          if (!ec)
            std::cout << "udp echo reply: " << readbuff << std::endl;

          socket_.close();
        });
  }

  void do_write(void) {
    std::string writbuf("Hello, this is udp echo client!");
    socket_.async_send(boost::asio::buffer(writbuf),
        [this](const boost::system::error_code& ec, std::size_t) {
          if (ec)
            socket_.close();
        });
  }
public:
  Client(boost::asio::io_service& io_service, std::uint16_t port,
      const char* remote_ip, std::uint16_t remote_port)
    : socket_(io_service, udp::endpoint(udp::v4(), port)) {
    do_connect(udp::endpoint(
          boost::asio::ip::address::from_string(remote_ip), remote_port));
  }
};

void run_client(void) {
  try {
    boost::asio::io_service io_service;

    udp::resolver r(io_service);
    Client c(io_service, 5656, "127.0.0.1", 5555);

    io_service.run();
  }
  catch (const std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
}
