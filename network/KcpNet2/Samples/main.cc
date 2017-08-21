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
#include <boost/asio.hpp>
#include "../KcpNet/KcpSession.h"
#include "../KcpNet/KcpClient.h"
#include "../KcpNet/KcpServer.h"

void run_client(void) {
  boost::asio::io_service io_service;

  KcpNet::KcpClient c(io_service, 5656);
  c.bind_connection_functor([](const KcpNet::KcpSessionPtr& s) {
        std::cout << "connect to 127.0.0.1:5555 success ..." << std::endl;
        s->write_buffer("Hello, world!");
      });
  c.bind_message_functor(
      [](const KcpNet::KcpSessionPtr& /*s*/, const std::string& buf) {
        std::cout << "from(127.0.0.1:5555) read: " << buf << std::endl;
      });

  c.connect("127.0.0.1", 5555);
  io_service.run();
}

void run_server(void) {
  boost::asio::io_service io_service;

  KcpNet::KcpServer s(io_service, 5555);
  s.bind_connection_functor([](const KcpNet::KcpSessionPtr& s) {
        std::cout
          << "accept session from: (" << s->get_endpoint()
          << ", " << s->get_conv() << ")" << std::endl;
      });
  s.bind_message_functor(
      [](const KcpNet::KcpSessionPtr& s, const std::string& buf) {
        std::cout
          << "from(" << s->get_endpoint() << ", " << s->get_conv()
          << ") read: " << buf << std::endl;
        s->write_buffer(buf);
      });

  io_service.run();
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  if (argc < 2) {
    std::cerr << "Usage: kcpnet [s|c] ..." << std::endl;
    return 0;
  }

  if (argv[1][0] == 's')
    run_server();
  else if (argv[1][0] == 'c')
    run_client();
  else
    std::cerr << "Usage: kcpnet [s|c] ..." << std::endl;

  return 0;
}
