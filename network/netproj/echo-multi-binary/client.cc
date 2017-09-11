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
#include <Chaos/IO/ColorIO.h>
#include "../base/netops.h"
#include "msg.h"

namespace cc = Chaos::ColorIO;

void run_client(void) {
  int sockfd = net::socket::open(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in addr{};
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5555);
  net::socket::connect(sockfd, (const struct sockaddr*)&addr);

  while (true) {
    RequestMsg msg;
    cc::printf(cc::ColorType::COLORTYPE_FG_RED, "please enter arg1 >>> ");
    std::cin >> msg.arg1;
    cc::printf(cc::ColorType::COLORTYPE_FG_RED, "please enter arg2 >>> ");
    std::cin >> msg.arg2;
    net::socket::write(sockfd, &msg, sizeof(msg));

    ResponseMsg res{};
    if (net::socket::read(sockfd, sizeof(res), &res) > 0) {
      std::cout << "from{127.0.0.1:5555} read: ";
      cc::printf(cc::ColorType::COLORTYPE_FG_CYAN, "%d\n", res.value);
    }
    else {
      break;
    }
  }

  net::socket::close(sockfd);
}
