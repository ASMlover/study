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
#include <Chaos/Concurrent/Thread.h>
#include "../libase/netops.h"

void run_server(void) {
  int sockfd = net::socket::open(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in6 host_addr{};
  host_addr.sin6_addr = in6addr_any;
  host_addr.sin6_family = AF_INET6;
  host_addr.sin6_port = htons(5555);
  net::socket::bind(sockfd, &host_addr, true);
  net::socket::listen(sockfd);

  std::vector<std::unique_ptr<Chaos::Thread>> threads;
  for (;;) {
    struct sockaddr_in6 addr{};
    int connfd = net::socket::accept(sockfd, &addr, true);
    std::cout
      << "accept connecting from: "
      << net::addr::as_string(&addr, true) << std::endl;
    auto t = new Chaos::Thread([connfd] {
          std::vector<char> buf(1024);
          for (;;) {
            auto n = net::socket::read(connfd, buf.size(), buf.data());
            if (n > 0)
              net::socket::write(connfd, buf.data(), n);
            else
              break;
          }
          net::socket::shutdown(connfd, net::socket::SHUT_WRITE);
          net::socket::close(connfd);
        });
    t->start();
    threads.emplace_back(t);
  }
  for (auto& t : threads)
    t->join();

  net::socket::close(sockfd);
}
