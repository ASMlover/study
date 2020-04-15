// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "./net/net.hh"
#include "examples.hh"

namespace echo_client {

using coro::net::Socket;

void launch() {
  coro::net::Initializer<> init;

  std::unique_ptr<Socket, std::function<void (Socket*)>> c{
      new Socket{}, [](Socket* s) { s->close(); }
  };

  if (!c->open() || !c->connect())
    return;

  char buf[1024];
  if (auto n = c->read(buf, sizeof(buf)); n != 1 || buf[0] != '*')
    return;

  coro::strv_t send_buf = "abcdefg^hijklmn$opq^rst$^uvw$xyz^000$";
  if (auto n = c->write(send_buf.data(), send_buf.size()); n == 0)
    return;
  std::cout << "CLIENT: send: " << send_buf << std::endl;

  coro::str_t rbuf;
  for (;;) {
    auto n = c->read(buf, sizeof(buf));
    if (n == 0)
      break;

    for (auto i = 0u; i < n; ++i)
      rbuf.push_back(buf[i]);
    if (rbuf.find("111") != std::string::npos)
      break;
  }
  std::cout << "CLIENT: recv: " << rbuf << std::endl;

  std::cout << "CLIENT: disconnecting ..." << std::endl;
}

}

CORO_EXAMPLE(EchoClient, ec, "an easy receiving client") {
  echo_client::launch();
}
