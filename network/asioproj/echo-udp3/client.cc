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
#include <vector>
#include <asio.hpp>

using asio::ip::udp;

class Client {
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  static constexpr std::size_t kBufferSize = 32 << 10;
  udp::socket socket_;
  std::vector<char> readbuff_;

  void do_connect(const char* remote_ip, std::uint16_t remote_port) {
    socket_.async_connect(
        udp::endpoint(asio::ip::address::from_string(remote_ip), remote_port),
        [this](const std::error_code& ec) {
          if (!ec) {
            do_read();
            write_echo();
          }
          else {
            socket_.close();
          }
        });
  }

  void do_read(void) {
    socket_.async_receive(asio::buffer(readbuff_),
        [this](const std::error_code& ec, std::size_t n) {
          if (!ec && n > 0) {
            std::cout << "echo reply: " << readbuff_.data() << std::endl;
            write_echo();
          }
          do_read();
        });
  }

  void write_echo(void) {
    static std::int64_t counter = 0;
    char buf[1024]{};
    auto n = std::snprintf(
        buf, sizeof(buf), "this is echo client, counter: %lld", ++counter);
    write(buf, n);
  }

  void write(const char* buf, std::size_t len) {
    socket_.async_send(asio::buffer(buf, len),
        [](const std::error_code& /*ec*/, std::size_t /*n*/) {});
  }
public:
  Client(asio::io_context& io_context, std::uint16_t port,
      const char* remote_ip, std::uint16_t remote_port)
    : socket_(io_context, udp::endpoint(udp::v4(), port))
    , readbuff_(kBufferSize) {
    do_connect(remote_ip, remote_port);
  }
};

void run_client(void) {
  try {
    asio::io_context io_context;

    Client c(io_context, 5656, "127.0.0.1", 5555);
    io_context.run();
  }
  catch (const std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
}
