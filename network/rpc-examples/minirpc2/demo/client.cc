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
#include "../minirpc/rpc_channel.h"
#include "../minirpc/rpc_handler.h"
#include "../minirpc/rpc_service.h"
#include "../minirpc/rpc_client.h"

using ::boost::asio::ip::tcp;

class MyHandler : public minirpc::RpcHandler {
  minirpc::RpcClient& rc_;
public:
  MyHandler(minirpc::RpcClient& rc)
    : rc_(rc) {
  }

  virtual void init_methods(void) override {
    REG_HANDLER_METHOD(MyHandler, back_hello);
  }

  void say_hello(const std::string& msg) {
    call_method_proxy("say_hello", msg);
  }

  void back_hello(const std::string& args) {
    std::cout << "MyHandler::back_hello - args=" << args << std::endl;

    rc_.close();
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  boost::asio::io_service io_service;

  minirpc::RpcClient client(io_service);
  MyHandler handler(client);
  client.start(&handler, [&handler] { handler.say_hello("Hello, world!"); });

  io_service.run();

  return 0;
}
