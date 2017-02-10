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
#include "echo.pb.h"
#include "tcp_connection.h"

TcpConnection::TcpConnection(tcp::socket&& socket)
  : socket_(std::move(socket))
  , buffer_(1024) {
}

TcpConnection::~TcpConnection(void) {
}

void TcpConnection::add_service(gpb::Service* service) {
  rpc_services_.push_back(service);
}

void TcpConnection::do_read(void) {
  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
        if (!ec) {
          handle_data(buffer_);

          do_read();
        }
        else {
          std::cerr << "read something error" << std::endl;
          socket_.close();
        }
      });
}

void TcpConnection::do_write(const char* buf, std::size_t len) {
  auto self(shared_from_this());
  boost::asio::async_write(socket_, boost::asio::buffer(buf, len),
      [this, self](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {
      });
}

void TcpConnection::CallMethod(const gpb::MethodDescriptor* method,
    gpb::RpcController* /*controller*/, const gpb::Message* request,
    gpb::Message* /*response*/, gpb::Closure* /*done*/) {
  char c[2]{};
  c[0] = '0' + method->index();
  c[1] = 0;
  std::string s(c);
  s += request->SerializeAsString();
  do_write(s.c_str(), s.size());
}

void TcpConnection::handle_data(std::vector<char>& buf) {
  echo::EchoRequest request;
  auto fid = buf[0] - '0';
  request.ParseFromString(&buf[1]);
  rpc_services_[0]->CallMethod(rpc_services_[0]->GetDescriptor()->method(fid), nullptr, &request, nullptr, nullptr);
}
