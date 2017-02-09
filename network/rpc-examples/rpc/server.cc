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
#include <vector>
#include <boost/asio.hpp>
#include "echo.pb.h"

using boost::asio::ip::tcp;
namespace gpb = google::protobuf;

class MyEchoService : public echo::EchoService {
public:
  void do_echo(gpb::RpcController* /*controller*/,
      const echo::EchoRequest* request, echo::EchoResponse* response, gpb::Closure* done) {
    std::cout << "MyEchoService::do_echo - request=" << request->request() << std::endl;
    response->set_response("recevied request");

    if (done)
      done->Run();
  }
};

class TcpSession : private boost::noncopyable, public std::enable_shared_from_this<TcpSession> {
  tcp::socket socket_;
  std::vector<char> buffer_;
  std::unique_ptr<MyEchoService> service_;
  echo::EchoRequest request_;
  echo::EchoResponse response_;

  enum { MAX_READBUFF = 1024 };

  void do_read(void) {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
        [this, self](const boost::system::error_code& ec, std::size_t n) {
          if (!ec) {
            request_.ParseFromArray(buffer_.data(), n);
            service_->do_echo(nullptr, &request_, &response_, nullptr);

            char writbuf[1024]{};
            response_.SerializeToArray(writbuf, sizeof(writbuf));
            do_write(writbuf, std::strlen(writbuf));
          }
          else {
            socket_.close();
          }
        });
  }

  void do_write(const char* buf, std::size_t len) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(buf, len),
        [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (ec)
            socket_.close();
        });
  }
public:
  TcpSession(tcp::socket&& socket)
    : socket_(std::move(socket))
    , buffer_(MAX_READBUFF)
    , service_(new MyEchoService()) {
  }

  void start(void) {
    do_read();
  }
};

class TcpServer : private boost::noncopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;

  void do_accept(void) {
    acceptor_.async_accept(socket_,
        [this](const boost::system::error_code& ec) {
          if (!ec)
            std::make_shared<TcpSession>(std::move(socket_))->start();

          do_accept();
        });
  }
public:
  TcpServer(boost::asio::io_service& io_service, std::uint16_t port = 5555)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    , socket_(io_service) {
  }

  void start(void) {
    do_accept();
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  try {
    boost::asio::io_service io_service;

    TcpServer server(io_service);
    server.start();

    io_service.run();
  }
  catch (std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }

  return 0;
}
