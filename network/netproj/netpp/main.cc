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
#include <iostream>
#include "buffer.h"
#include "primitive.h"
#include "address.h"
#include "socket.h"

#if defined(CHAOS_POSIX)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

void echo_server(void) {
  std::error_code ec;

  netpp::TcpSocket s;
  s.open(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  netpp::Address host_addr6(5555, false, true);
  s.bind(host_addr6);
  netpp::socket::listen(s.get_fd(), ec);

  std::vector<std::unique_ptr<std::thread>> threads;
  for (;;) {
    struct sockaddr_in6 addr6{};
    int connfd = netpp::socket::accept(s.get_fd(), &addr6, ec, true);
    threads.emplace_back(new std::thread([connfd] {
            std::error_code ec;
            std::vector<char> buf(1024);
            for (;;) {
              auto n = netpp::socket::read(connfd, buf.size(), buf.data(), ec);
              if (n > 0)
                netpp::socket::write(connfd, buf.data(), n, ec);
              else
                break;
            }
            netpp::socket::close(connfd, ec);
          }));
  }
  for (auto& t : threads)
    t->join();

  s.close();
}

void echo_client(void) {
  std::error_code ec;

  netpp::TcpSocket s;
  s.open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  s.connect(netpp::Address("127.0.0.1", 5555));

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

void echo_sample(char c) {
  if (c == 's')
    echo_server();
  else if (c == 'c')
    echo_client();
}
#else
void echo_sample(char c) {
  std::cout << "netpp - echo_sample: c=" << c << std::endl;
}
#endif

int main(int argc, char* argv[]) {
  CHAOS_UNUSED(argc), CHAOS_UNUSED(argv);

  if (argc > 1)
    echo_sample(argv[1][0]);

  return 0;
}
