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
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
const int MAX_LENGTH = 1024;

void run_client(const char* remote_addr = "127.0.0.1", const char* remote_port = "5555") {
  boost::asio::io_service io_service;
  udp::socket s(io_service, udp::endpoint(udp::v4(), 0));
  udp::resolver r(io_service);
  auto endpoint = *r.resolve({udp::v4(), remote_addr, remote_port});

  const char* send_buff = "This is udp echo client";
  s.send_to(boost::asio::buffer(send_buff, std::strlen(send_buff)), endpoint);

  udp::endpoint sender_ep;
  char buff[MAX_LENGTH];
  s.receive_from(boost::asio::buffer(buff, MAX_LENGTH), sender_ep);
  std::cout << "Receive: " << buff << std::endl;
}
