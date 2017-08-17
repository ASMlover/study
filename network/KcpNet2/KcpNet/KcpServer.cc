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
#include "Utility.h"
#include "KcpServer.h"

namespace KcpNet {

KcpServer::KcpServer(boost::asio::io_service& io_service, std::uint16_t port)
  : socket_(io_service, udp::endpoint(udp::v4(), port))
  , readbuff_(kBufferSize) {
  do_read();
}

void KcpServer::do_read(void) {
  socket_.async_receive_from(boost::asio::buffer(readbuff_), sender_ep_,
      [this](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && n > 0) {
          std::cout << "from: " << sender_ep_ << ", read: " << readbuff_.data() << std::endl;
          if (is_connect_request(readbuff_.data(), n)) {
            // response the connect request
            write(make_connect_response(0), sender_ep_);
          }
          else {
            // TODO: solve received message
          }
        }

        do_read();
      });
}

void KcpServer::write(const std::string& buf, const udp::endpoint& ep) {
  socket_.async_send_to(boost::asio::buffer(buf), ep,
      [](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {});
}

void KcpServer::write(
    const char* buf, std::size_t len, const udp::endpoint& ep) {
  socket_.async_send_to(boost::asio::buffer(buf, len), ep,
      [](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {});
}

}
