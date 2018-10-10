// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "TcpSession.h"
#include "TcpClient.h"

namespace nyx {

TcpClient::TcpClient(asio::io_context& context) {
  tcp::socket socket(context);
  conn_.reset(new TcpSession(std::move(socket)));
}

TcpClient::~TcpClient(void) {
}

void TcpClient::connect(const char* host, std::uint16_t port) {
  tcp::endpoint ep(asio::ip::address::from_string(host), port);
  auto conn = dynamic_cast<TcpSession*>(conn_.get());
  conn->get_socket().async_connect(ep, [this, conn](const std::error_code& ec) {
        if (!ec) {
          if (connected_fn_)
            connected_fn_(conn_);
          conn->set_message_callback(message_fn_);
          conn->set_closed_callback(closed_fn_);
          conn->do_read();
        }
      });
}

void TcpClient::write(const std::string& buf) {
  conn_->write(buf);
}

void TcpClient::close(void) {
  conn_->disconnect();
}

}
