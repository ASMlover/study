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

using boost::asio::ip::tcp;

class ChatClient : private boost::noncopyable {
  tcp::socket socket_;
  std::vector<char> buffer_;

  void do_read(void) {
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [this](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec)
            std::cout << "receive from server: " << buffer_.data() << std::endl;

          socket_.close();
        });
  }

  void do_write(void) {
    std::string send_msg("This is chat client");
    boost::asio::async_write(socket_, boost::asio::buffer(send_msg),
        [this](const boost::system::error_code& ec, std::size_t /*n*/) {
          std::cout << "ChatClient::do_write - write complete handler coming ..." << std::endl;
          if (ec)
            socket_.close();
        });
  }
public:
  ChatClient(boost::asio::io_service& io_service)
    : socket_(io_service) {
  }

  void start(const char* host = "127.0.0.1", const char* port = "5555") {
    std::cout << "begin connect to {" << host << ", " << port << "} ..." << std::endl;
    tcp::resolver r(socket_.get_io_service());
    boost::asio::async_connect(socket_, r.resolve({host, port}),
        [this, host, port](const boost::system::error_code& ec, tcp::resolver::iterator /*endpoint*/) {
          if (!ec) {
            std::cout << "connect to {" << host << ", " << port << "} success ..." << std::endl;
            do_read();
            do_write();
          }
        });
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  boost::asio::io_service io_service;

  ChatClient clint(io_service);
  clint.start();

  io_service.run();

  return 0;
}
