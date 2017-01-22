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
#include <cstddef>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/yield.hpp>

using boost::asio::ip::tcp;

class EchoServer : private boost::noncopyable, private boost::asio::coroutine {
  tcp::acceptor& acceptor_;
  tcp::socket socket_;
  std::vector<char> buff_;

  class _Ref {
    EchoServer* es_{};
  public:
    explicit _Ref(EchoServer* es)
      : es_(es) {
    }

    void operator()(boost::system::error_code ec, std::size_t n = 0) {
      (*es_)(ec, n);
    }
  };
public:
  EchoServer(tcp::acceptor& acceptor, std::size_t bufsize = 1024)
    : acceptor_(acceptor)
    , socket_(acceptor.get_io_service())
    , buff_(bufsize) {
  }

  void operator()(boost::system::error_code ec = boost::system::error_code(), std::size_t n = 0) {
    reenter (this) for (;;) {
      yield acceptor_.async_accept(socket_, _Ref(this));

      while (!ec) {
        yield socket_.async_read_some(boost::asio::buffer(buff_), _Ref(this));

        if (!ec)
          yield boost::asio::async_write(socket_, boost::asio::buffer(buff_, n), _Ref(this));
      }

      socket_.close();
    }
  }
};

int main(int argc, char* argv[]) {
  ((void)argc), ((void)argv);

  boost::asio::io_service io;
  tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 5555));
  std::vector<std::shared_ptr<EchoServer>> servers;

  for (int i = 0; i < 4; ++i) {
    std::shared_ptr<EchoServer> s(new EchoServer(acceptor));
    servers.push_back(s);
    (*s)();
  }

  io.run();

  return 0;
}
