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
#include <string>
#include <vector>
#include <unordered_set>
#include "../libase/netops.h"

void run_server(void) {
  int sockfd = net::socket::open(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in host_addr{};
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(5555);
  net::socket::bind(sockfd, &host_addr);
  net::socket::listen(sockfd);

  std::unordered_set<int> clients;

  fd_set rset_all;
  fd_set rset;
  FD_ZERO(&rset_all);
  FD_SET(sockfd, &rset_all);

  auto maxfd = sockfd;
  for (;;) {
    rset = rset_all;
    auto count = net::io::select(maxfd + 1, &rset, nullptr, nullptr, nullptr);

    if (FD_ISSET(sockfd, &rset)) {
      struct sockaddr_in addr{};
      int connfd = net::socket::accept(sockfd, &addr);

      if (clients.size() < FD_SETSIZE)
        clients.insert(connfd);
      else
        break;

      FD_SET(connfd, &rset_all);
      if (connfd > maxfd)
        maxfd = connfd;
      if (--count <= 0)
        continue;
    }

    for (auto it = clients.begin(); it != clients.end() && count > 0; --count) {
      auto fd = *it;
      if (FD_ISSET(fd, &rset)) {
        std::vector<char> buf(1024);
        auto n = net::socket::read(fd, buf.size(), buf.data());
        if (n > 0) {
          net::socket::write(fd, buf.data(), n);
          ++it;
        }
        else {
          net::socket::shutdown(fd, net::socket::SHUT_BOTH);
          net::socket::close(fd);
          FD_CLR(fd, &rset_all);

          clients.erase(it++);
        }
      }
    }
  }

  net::socket::close(sockfd);
}
