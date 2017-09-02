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
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class Server : private boost::noncopyable {
  udp::socket socket_;
  udp::endpoint sender_ep_;

  static constexpr std::size_t kMaxBufferSize = 1024;
  std::vector<char> readbuff_;

  void do_read(void) {
    socket_.async_receive_from(boost::asio::buffer(readbuff_), sender_ep_,
        [this](const boost::system::error_code& ec, std::size_t read_bytes) {
          if (!ec && read_bytes > 0) {
            std::cout
              << "from: " << sender_ep_
              << ", read buffer: " << readbuff_.data() << std::endl;
            do_write(read_bytes);
          }
          else {
            do_read();
          }
        });
  }

  void do_write(std::size_t length) {
    socket_.async_send_to(boost::asio::buffer(readbuff_, length), sender_ep_,
        [this](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {
          do_read();
        });
  }
public:
  Server(boost::asio::io_service& io_service, std::uint16_t port)
    : socket_(io_service, udp::endpoint(udp::v4(), port))
    , readbuff_(kMaxBufferSize) {
    do_read();
  }
};

void run_server(void) {
  try {
    boost::asio::io_service io_service;

    Server s(io_service, 5555);
    io_service.run();
  }
  catch (const std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
}
