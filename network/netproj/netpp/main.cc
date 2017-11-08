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
#include <Chaos/Base/Platform.h>
#include <Chaos/Base/Types.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "buffer.h"
#include "primitive.h"
#include "address.h"
#include "socket.h"
#include "acceptor.h"
#include "socket_service.h"

class TcpConnection
  : private Chaos::UnCopyable
  , public std::enable_shared_from_this<TcpConnection> {
  netpp::TcpSocket socket_;
  std::vector<char> buffer_;

  static constexpr std::size_t kMaxBuffer = 1024;

  void do_read(void) {
    auto self(shared_from_this());
    socket_.async_read_some(netpp::buffer(buffer_),
        [this, self](const std::error_code& ec, std::size_t n) {
          if (!ec)
            do_write(n);
        });
  }

  void do_write(std::size_t n) {
    auto self(shared_from_this());
    socket_.async_write_some(netpp::buffer(buffer_, n),
        [this, self](const std::error_code& ec, std::size_t) {
          if (!ec)
            do_read();
        });
  }
public:
  TcpConnection(netpp::TcpSocket&& s)
    : socket_(std::move(s))
    , buffer_(kMaxBuffer) {
  }

  void start(void) {
    do_read();
  }
};

class EchoTcpServer : private Chaos::UnCopyable {
  netpp::Acceptor acceptor_;
  netpp::TcpSocket socket_;

  void do_accept(void) {
    acceptor_.async_accept(socket_, [this](const std::error_code& ec) {
          if (!ec)
            std::make_shared<TcpConnection>(std::move(socket_));

          do_accept();
        });
  }
public:
  EchoTcpServer(netpp::SocketService& service, std::uint16_t port)
#if defined(CHAOS_POSIX)
    : acceptor_(service, netpp::Address(netpp::IP::v6(), port))
#else
    : acceptor_(service, netpp::Address(netpp::IP::v4(), port))
#endif
    , socket_(service) {
    acceptor_.set_non_blocking(true);
  }

  void start(void) {
    do_accept();
  }
};

void echo_tcp_server(void) {
  netpp::SocketService service;
  EchoTcpServer server(service, 5555);
  server.start();

// #if defined(CHAOS_POSIX)
//   netpp::Acceptor acceptor(service, netpp::Address(netpp::IP::v6(), 5555));
// #else
//   netpp::Acceptor acceptor(service, netpp::Address(netpp::IP::v4(), 5555));
// #endif
//   acceptor.set_non_blocking(true);
//   std::vector<std::unique_ptr<std::thread>> threads;
//   netpp::TcpSocket conn(service);
//   acceptor.async_accept(conn,
//       [&threads, &acceptor, &conn](const std::error_code& ec) {
//         if (!ec) {
//           threads.emplace_back(new std::thread([](netpp::TcpSocket&& conn) {
//                   std::error_code ec;
//                   std::vector<char> buf(1024);
//                   for (;;) {
//                     auto n = conn.read_some(netpp::buffer(buf));
//                     if (n > 0)
//                       conn.write_some(netpp::buffer(buf, n));
//                     else
//                       break;
//                   }
//                   conn.close();
//                 }, std::move(conn)));
//         }
//       });

  service.run();

//   for (auto& t : threads)
//     t->join();
}

void echo_tcp_client(void) {
  netpp::SocketService service;
  netpp::TcpSocket s(service, netpp::Tcp::v4());
  s.set_non_blocking(true);
  s.connect(netpp::Address(netpp::IP::v4(), "127.0.0.1", 5555));

  std::string line;
  std::vector<char> buf(1024);
  while (std::getline(std::cin, line)) {
    if (line == "quit")
      break;
    s.write(netpp::buffer(line));

    buf.assign(buf.size(), 0);
    if (s.read(netpp::buffer(buf)) > 0)
      std::cout << "echo read: " << buf.data() << std::endl;
    else
      break;
  }
  s.close();
}

void echo_udp_server(void) {
  netpp::SocketService service;
  netpp::UdpSocket s(service, netpp::Address(netpp::IP::v4(), 5555));
  s.set_non_blocking(true);

  std::vector<char> buf(1024);
  for (;;) {
    netpp::Address addr(netpp::IP::v4());
    auto n = s.read_from(netpp::buffer(buf), addr);
    if (n > 0)
      s.write_to(netpp::buffer(buf, n), addr);
    else
      break;
  }
  s.close();
}

void echo_udp_client(void) {
  netpp::SocketService service;
  netpp::UdpSocket s(service, netpp::UdpSocket::ProtocolType::v4());
  s.set_non_blocking(true);
  s.connect(netpp::Address(netpp::IP::v4(), "127.0.0.1", 5555));

  std::string line;
  std::vector<char> buf(1024);
  while (std::getline(std::cin, line)) {
    if (line == "quit")
      break;
    s.write(netpp::buffer(line));

    buf.assign(buf.size(), 0);
    if (s.read(netpp::buffer(buf)) > 0)
      std::cout << "from{127.0.0.1:5555} read: " << buf.data() << std::endl;
  }
  s.close();
}

void echo_sample(const char* c) {
  netpp::startup();
  if (std::strcmp(c, "ts") == 0)
    echo_tcp_server();
  else if (std::strcmp(c, "tc") == 0)
    echo_tcp_client();
  else if (std::strcmp(c, "us") == 0)
    echo_udp_server();
  else if (std::strcmp(c, "uc") == 0)
    echo_udp_client();
  netpp::cleanup();
}

int main(int argc, char* argv[]) {
  CHAOS_UNUSED(argc), CHAOS_UNUSED(argv);

  if (argc > 1)
    echo_sample(argv[1]);

  return 0;
}
