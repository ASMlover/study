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
#include "echo.pb.h"

using boost::asio::ip::tcp;
namespace gpb = google::protobuf;

class MyRpcChannel : public gpb::RpcChannel {
  tcp::socket& client_socket_;
public:
  MyRpcChannel(tcp::socket& client_socket)
    : client_socket_(client_socket) {
  }

  virtual void CallMethod(const gpb::MethodDescriptor* method, gpb::RpcController* /*controller*/,
      const gpb::Message* request, gpb::Message* response, gpb::Closure* done) override {
    std::cout << "MyRpcChannel::CallMethodi - " << method->DebugString() << std::endl;

    char buf[1024]{};
    request->SerializeToArray(buf, sizeof(buf));
    client_socket_.write_some(boost::asio::buffer(buf, std::strlen(buf)));

    std::memset(buf, 0, sizeof(buf));
    std::size_t n = client_socket_.read_some(boost::asio::buffer(buf, sizeof(buf)));
    response->ParseFromArray(buf, n);

    if (done)
      done->Run();
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  try {
    boost::asio::io_service io_service;

    tcp::socket socket(io_service);
    socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 5555));

    MyRpcChannel channel(socket);
    echo::EchoService::Stub stub(&channel);

    echo::EchoRequest request;
    request.set_request("Hello, world! This is RPC.CLIENT");
    echo::EchoResponse response;
    stub.do_echo(nullptr, &request, &response, nullptr);

    std::cout << "Get echo response : " << response.response() << std::endl;

    io_service.run();
  }
  catch (std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }

  return 0;
}
