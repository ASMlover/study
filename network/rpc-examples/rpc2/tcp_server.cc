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
#include "rpc_service.h"
#include "tcp_server.h"

TcpServer::TcpServer(boost::asio::io_service& io_service, std::uint16_t port)
  : io_service_(io_service)
  , acceptor_(io_service_, tcp::endpoint(tcp::v4(), port))
  , socket_(io_service_) {
  do_accept();
}

void TcpServer::do_echo(const char* buf) {
  for (auto& conn : connections_) {
    echo::EchoRequest request;
    request.set_request(buf);
    echo::EchoService::Stub stub(conn.get());
    stub.do_echo(nullptr, &request, nullptr, nullptr);
  }
}

void TcpServer::do_accept(void) {
  acceptor_.async_accept(socket_,
      [this](const boost::system::error_code& ec) {
        if (!ec) {
          auto conn = std::make_shared<TcpConnection>(std::move(socket_));
          conn->add_service(new RpcEchoResponseService(conn.get()));
          connections_.push_back(conn);

          conn->do_read();
        }

        do_accept();
      });
}
