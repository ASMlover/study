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
#include <algorithm>
#include <memory>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ChatSession : public std::enable_shared_from_this<ChatSession> {
  static const int kDefBufferBytes = 1024;

  tcp::socket socket_;
  std::vector<char> buffer_;

  void do_start(void) {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [this, self](const boost::system::error_code& ec, std::size_t n) {
          if (!ec)
            do_write(n);
          else
            socket_.close();
        });
  }

  void do_write(std::size_t written_bytes) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(buffer_, written_bytes),
        [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec)
            ;
          else
            socket_.close();
        });
  }
public:
  ChatSession(tcp::socket&& socket)
    : socket_(std::move(socket))
    , buffer_(kDefBufferBytes) {
  }

  tcp::socket& get_socket(void) {
    return socket_;
  }

  void start(void) {
    do_start();
  }
};

class ChatServer : private boost::noncopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;

  void do_accept(void) {
    acceptor_.async_accept(socket_,
        [this](const boost::system::error_code& ec) {
          if (!ec)
            std::make_shared<ChatSession>(std::move(socket_))->start();

          do_accept();
        });
  }
public:
  ChatServer(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
    : acceptor_(io_service, endpoint)
    , socket_(io_service) {
  }

  void start(void) {
    do_accept();
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  boost::asio::io_service io_service;

  ChatServer server(io_service, tcp::endpoint(tcp::v4(), 5555));
  server.start();

  io_service.run();

  return 0;
}
