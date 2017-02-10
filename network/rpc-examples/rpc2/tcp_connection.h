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

#include <memory>
#include <vector>
#include <boost/asio.hpp>
#include <google/protobuf/service.h>

using boost::asio::ip::tcp;
namespace gpb = google::protobuf;

class TcpConnection : public gpb::RpcChannel, public std::enable_shared_from_this<TcpConnection> {
  tcp::socket socket_;
  std::vector<char> buffer_;
  std::vector<gpb::Service*> rpc_services_;

  void handle_data(std::vector<char>& buf);
public:
  explicit TcpConnection(tcp::socket&& socket);
  ~TcpConnection(void);

  void add_service(gpb::Service* service);
  void do_read(void);
  void do_write(const char* buf, std::size_t len);

  virtual void CallMethod(const gpb::MethodDescriptor* method, gpb::RpcController* controller,
      const gpb::Message* request, gpb::Message* response, gpb::Closure* done) override;

  tcp::socket& get_socket(void) {
    return socket_;
  }
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
