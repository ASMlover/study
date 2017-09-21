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
#include <vector>
#include "../libase/netops.h"

void run_server(void) {
  int sockfd = net::socket::open(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in host_addr{};
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(5555);
  net::socket::bind(sockfd, &host_addr);
  net::socket::listen(sockfd);

  std::vector<pollfd> clients;
  std::vector<char> buf(1024);
  {
    pollfd c{};
    c.fd = sockfd;
    c.events = POLLRDNORM;
    clients.push_back(c);
  }
  for (;;) {
    auto nready = net::io::poll(clients.data(),
        static_cast<std::uint32_t>(clients.size()), -1);

    if (clients[0].revents & POLLRDNORM) {
      struct sockaddr_in addr{};
      int connfd = net::socket::accept(sockfd, &addr);
      pollfd c{};
      c.fd = connfd;
      c.events = POLLRDNORM;
      clients.push_back(c);

      if (--nready <= 0)
        continue;
    }
    for (auto& c : clients) {
      int fd = static_cast<int>(c.fd);
      if (fd == sockfd)
        continue;

      if (c.revents & (POLLRDNORM | POLLERR)) {
        buf.assign(buf.size(), 0);
        auto n = net::socket::read(fd, buf.size(), buf.data());
        if (n > 0) {
          net::socket::write(fd, buf.data(), n);
        }
        else {
          net::socket::shutdown(fd, net::socket::SHUT_BOTH);
          net::socket::close(fd);

          auto it = std::find_if(
              clients.begin(), clients.end(), [&fd](const pollfd& c) {
                return static_cast<int>(c.fd) == fd;
              });
          if (it != clients.end())
            clients.erase(it);
        }

        if (--nready <= 0)
          break;
      }
    }
  }
  net::socket::close(sockfd);
}
