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

class tcp_client : private boost::noncopyable {
  tcp::socket socket_;
  tcp::resolver::iterator epiter_;

public:
  tcp_client(boost::asio::io_service& io_service, tcp::resolver::iterator epiter)
    : socket_(io_service)
    , epiter_(epiter) {
  }

  void start(void) {
    boost::asio::async_connect(socket_, epiter_,
        [this](const boost::system::error_code& ec, tcp::resolver::iterator) {
          if (!ec) {
            std::vector<char> buf(1024);
            socket_.async_read_some(boost::asio::buffer(buf),
                [this, &buf](const boost::system::error_code& ec, std::size_t /*n*/) {
                  if (!ec)
                    std::cout << "daytime client - " << buf.data() << std::endl;

                  socket_.close();
                });
          }
          else {
            socket_.close();
          }
        });
  }
};

void start_sync_client(void) {
  boost::asio::io_service io_service;
  tcp::socket client_socket(io_service);
  client_socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 5555));

  std::vector<char> buf(1024);
  client_socket.read_some(boost::asio::buffer(buf));

  std::cout << "get daytime is : " << buf.data() << std::endl;

  client_socket.close();
}

void start_async_client(void) {
  boost::asio::io_service io_service;

  tcp::resolver r(io_service);
  tcp_client client(io_service, r.resolve({"127.0.0.1", "5555"}));
  client.start();

  io_service.run();
}

int main(int argc, char* argv[]) {
  (void)argc; (void)argv;

  if (argc != 2) {
    std::cerr << "Usage ./asio.client [async|sync] ..." << std::endl;
    return 0;
  }

  if (std::string("async") == argv[1])
    start_async_client();
  else
    start_sync_client();

  return 0;
}
