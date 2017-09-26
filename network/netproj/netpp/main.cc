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
#include <Chaos/Base/Types.h>
#include "buffer.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "primitive.h"

void echo_server(void) {
  int sockfd = netpp::socket::open(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in6 host_addr6{};
  host_addr6.sin6_addr = in6addr_any;
  host_addr6.sin6_family = AF_INET6;
  host_addr6.sin6_port = htons(5555);
  netpp::socket::bind(sockfd, &host_addr6);
  netpp::socket::listen(sockfd);

  std::vector<std::unique_ptr<std::thread>> threads;
  for (;;) {
    struct sockaddr_in6 addr6{};
    int connfd = netpp::socket::accept(sockfd, &addr6, true);
    threads.emplace_back(new std::thread([connfd] {
            std::vector<char> buf(1024);
            for (;;) {
              auto n = netpp::socket::read(connfd, buf.size(), buf.data());
              if (n > 0)
                netpp::socket::write(connfd, buf.data(), n);
              else
                break;
            }
            netpp::socket::close(connfd);
          }));
  }
  for (auto& t : threads)
    t->join();

  netpp::socket::close(sockfd);
}

void echo_client(void) {
  int sockfd = netpp::socket::open(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in addr{};
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5555);
  netpp::socket::connect(sockfd, &addr);

  std::string line;
  std::vector<char> buf(1024);
  while (std::getline(std::cin, line)) {
    if (line == "quit")
      break;
    netpp::socket::write(sockfd, line.data(), line.size());

    buf.assign(buf.size(), 0);
    if (netpp::socket::read(sockfd, buf.size(), buf.data()) > 0)
      std::cout << "echo read: " << buf.data() << std::endl;
    else
      break;
  }

  netpp::socket::close(sockfd);
}

int main(int argc, char* argv[]) {
  CHAOS_UNUSED(argc), CHAOS_UNUSED(argv);

  if (argv[1][0] == 's')
    echo_server();
  if (argv[1][0] == 'c')
    echo_client();

  return 0;
}
