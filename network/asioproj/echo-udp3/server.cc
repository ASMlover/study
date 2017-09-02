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

class Server {
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  static constexpr std::size_t kBufferSize = 32 << 10;
  udp::socket socket_;
  udp::endpoint sender_ep_;
  std::vector<char> readbuff_;

  void do_read(void) {
    socket_.async_receive_from(asio::buffer(readbuff_), sender_ep_,
        [this](const std::error_code& ec, std::size_t n) {
          if (!ec && n > 0) {
            std::cout
              << "from: " << sender_ep_ << ", read: " << readbuff_.data()
              << std::endl;
            write(readbuff_.data(), n, sender_ep_);
          }

          do_read();
        });
  }

  void write(const char* buf, std::size_t len, const udp::endpoint& ep) {
    socket_.async_send_to(asio::buffer(buf, len), ep,
        [](const std::error_code& /*ec*/, std::size_t /*n*/) {});
  }
public:
  Server(asio::io_context& io_context, std::uint16_t port)
    : socket_(io_context, udp::endpoint(udp::v4(), port))
    , readbuff_(kBufferSize) {
    do_read();
  }
};

void run_server(void) {
  try {
    asio::io_context io_context;

    Server s(io_context, 5555);
    io_context.run();
  }
  catch (const std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
}
