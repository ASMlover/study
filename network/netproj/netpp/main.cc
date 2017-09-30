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
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "netpp_internal.h"
#include "buffer.h"
#include "primitive.h"
#include "address.h"
#include "socket.h"
#include "acceptor.h"

void echo_server(void) {
#if defined(CHAOS_POSIX)
  netpp::Acceptor acceptor(netpp::Address(netpp::IP::v6(), 5555));
#else
  netpp::Acceptor acceptor(netpp::Address(netpp::IP::v4(), 5555));
#endif
  std::vector<std::unique_ptr<std::thread>> threads;
  for (;;) {
    netpp::TcpSocket conn;
    acceptor.accept(conn);
    threads.emplace_back(new std::thread([](netpp::TcpSocket&& conn) {
            std::error_code ec;
            std::vector<char> buf(1024);
            for (;;) {
              auto n = conn.read(netpp::buffer(buf));
              if (n > 0)
                conn.write(netpp::buffer(buf, n));
              else
                break;
            }
            conn.close();
          }, std::move(conn)));
  }
  for (auto& t : threads)
    t->join();

  acceptor.close();
}

void echo_client(void) {
  netpp::TcpSocket s(netpp::Tcp::v4());
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

void echo_sample(char c) {
  netpp::startup();
  if (c == 's')
    echo_server();
  else if (c == 'c')
    echo_client();
  netpp::cleanup();
}

int main(int argc, char* argv[]) {
  CHAOS_UNUSED(argc), CHAOS_UNUSED(argv);

  if (argc > 1)
    echo_sample(argv[1][0]);

  return 0;
}
