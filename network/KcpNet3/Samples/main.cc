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
#include <asio.hpp>
#include "../KcpNet/Session.h"
#include "../KcpNet/Client.h"
#include "../KcpNet/Server.h"

void run_client(void) {
  asio::io_context io_context;

  KcpNet::Client c(io_context, 5656);
  c.bind_error_functor([](void) {
        std::cout << "connect to(127.0.0.1:5555) failed ..." << std::endl;
      });
  c.bind_connecttion_functor([](const KcpNet::SessionPtr& s) {
        std::cout << "connect to(127.0.0.1:5555) success ..." << std::endl;
        s->write("Hello, world!");
      });
  c.bind_message_functor(
      [](const KcpNet::SessionPtr& /*s*/, const std::string& buf) {
        std::cout << "from(127.0.0.1:5555) read: " << buf << std::endl;
      });
  c.connect("127.0.0.1", 5555);

  io_context.run();
}

void run_server(void) {
  asio::io_context io_context;

  KcpNet::Server s(io_context, 5555);
  s.bind_connecttion_functor([](const KcpNet::SessionPtr& sp) {
        std::cout
          << "accept session from {" << sp->get_endpoint()
          << ", " << sp->get_conv() << "}" << std::endl;
      });
  s.bind_message_functor(
      [](const KcpNet::SessionPtr& sp, const std::string& buf) {
        std::cout
          << "from{" << sp->get_endpoint() << "," << sp->get_conv() << "} "
          << "read: " << buf << std::endl;
        sp->write(buf);
      });

  io_context.run();
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  if (argc < 2) {
    std::cout << "USAGE: kcpnet-samples [s|c] ..." << std::endl;
    return 0;
  }

  if (argv[1][0] == 's')
    run_server();
  else if (argv[1][0] == 'c')
    run_client();
  else
    std::cout << "USAGE: kcpnet-samples [s|c] ..." << std::endl;

  return 0;
}
