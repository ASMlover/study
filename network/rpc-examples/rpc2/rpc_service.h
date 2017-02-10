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
#pragma once

#include <iostream>
#include "echo.pb.h"
#include "tcp_connection.h"

class RpcEchoService : public echo::EchoService {
  TcpConnection* conn_;
public:
  RpcEchoService(TcpConnection* conn)
    : conn_(conn) {
    conn_->add_service(this);
  }
};

class RpcEchoRequestService : public RpcEchoService {
public:
  RpcEchoRequestService(TcpConnection* conn)
    : RpcEchoService(conn) {
  }

  void do_echo(gpb::RpcController* /*controller*/,
      const echo::EchoRequest* request, echo::EchoResponse* /*response*/, gpb::Closure* /*done*/) {
    std::cout << "RPC, RpcEchoResponseService::do_echo - message=" << request->request() << std::endl;
  }
};

class RpcEchoResponseService : public RpcEchoService {
  std::unique_ptr<echo::EchoService::Stub> stub_sercice_;
public:
  RpcEchoResponseService(TcpConnection* conn)
    : RpcEchoService(conn)
    , stub_sercice_(new echo::EchoService::Stub(conn)) {
  }

  void do_echo(gpb::RpcController* /*controller*/,
      const echo::EchoRequest* request, echo::EchoResponse* /*response*/, gpb::Closure* /*done*/) {
    std::cout << "RPC, RpcEchoResponseService::do_echo - message=" << request->request() << std::endl;

    echo::EchoRequest back;
    back.set_request(request->request());
    stub_sercice_->do_echo(nullptr, &back, nullptr, nullptr);
  }
};
