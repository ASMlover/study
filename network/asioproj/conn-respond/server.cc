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
#include <string>
#include <utility>
#include <vector>
#include <boost/asio.hpp>

typedef boost::asio::ip::tcp::acceptor AcceptorT;
typedef boost::asio::ip::tcp::socket SocketT;

class SharedConstBuffer {
  std::shared_ptr<std::vector<char>> data_;
  boost::asio::const_buffer buffer_;
public:
  typedef boost::asio::const_buffer value_type;
  typedef const boost::asio::const_buffer* const_iterator;

  explicit SharedConstBuffer(const std::string& data)
    : data_(new std::vector<char>(data.begin(), data.end()))
    , buffer_(boost::asio::buffer(*data_)) {
  }

  const boost::asio::const_buffer* begin(void) const {
    return &buffer_;
  }

  const boost::asio::const_buffer* end(void) const {
    return &buffer_ + 1;
  }
};

class Session : public std::enable_shared_from_this<Session> {
  SocketT socket_;

  void do_write(void) {
    std::time_t now = std::time(nullptr);
    SharedConstBuffer buf(std::ctime(&now));
    auto self(shared_from_this());
    boost::asio::async_write(socket_, buf,
        [self](boost::system::error_code, std::size_t) {
        });
  }
public:
  Session(SocketT&& s)
    : socket_(std::move(s)) {
  }

  void start(void) {
    do_write();
  }
};

class Server {
  AcceptorT acceptor_;
  SocketT socket_;

  void do_accept(void) {
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
      if (!ec) {
        std::cout << "get a new connection" << std::endl;
        std::make_shared<Session>(std::move(socket_))->start();
      }

      do_accept();
    });
  }
public:
  Server(boost::asio::io_service& service, std::uint16_t port)
    : acceptor_(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , socket_(service) {
    do_accept();
  }
};

int main(int argc, char* argv[]) {
  boost::asio::io_service service;
  Server s(service, 5555);
  service.run();

  return 0;
}
