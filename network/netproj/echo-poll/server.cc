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
#include <memory>
#include <vector>
#include <Chaos/Concurrent/Thread.h>
#include "../base/netops.h"

void run_server(void) {
  struct sockaddr_in host_addr{};
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(5555);

  int reuse = 1;
  int tcpfd = net::socket::open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  net::socket::set_option(tcpfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
  net::socket::bind(tcpfd, &host_addr);
  net::socket::listen(tcpfd);

  int udpfd = net::socket::open(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  net::socket::bind(udpfd, &host_addr);

  std::vector<std::unique_ptr<Chaos::Thread>> threads;

  std::vector<pollfd> clients;
  auto new_pollfd = [](int fd) -> pollfd {
    pollfd c{};
    c.fd = fd;
    c.events = POLLRDNORM;
    return c;
  };
  clients.push_back(new_pollfd(tcpfd));
  clients.push_back(new_pollfd(udpfd));
  for (;;) {
    auto nready = net::io::poll(clients.data(),
        (std::uint32_t)clients.size(), -1);
    if (nready <= 0)
      break;

    if (clients[0].revents & POLLRDNORM) {
      struct sockaddr_in addr{};
      int connfd = net::socket::accept(tcpfd, &addr);
      auto t = new Chaos::Thread([connfd] {
            std::vector<char> buf(1024);
            for (;;) {
              auto n = net::socket::read(connfd, buf.size(), buf.data());
              if (n > 0)
                net::socket::write(connfd, buf.data(), n);
              else
                break;
            }
            net::socket::close(connfd);
          });
      t->start();
      threads.emplace_back(t);
    }

    if (clients[1].revents & POLLRDNORM) {
      struct sockaddr_in addr{};
      std::vector<char> buf(1024);
      auto n = net::socket::readfrom(udpfd, buf.size(), buf.data(), &addr);
      if (n > 0)
        net::socket::writeto(udpfd, buf.data(), n, &addr);
    }
  }
  for (auto& t : threads)
    t->join();

  net::socket::close(udpfd);
  net::socket::close(tcpfd);
}
