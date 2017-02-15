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
#include "rpc_channel.h"
#include "rpc_handler.h"
#include "rpc_service.h"
#include "rpc_client.h"
#include <iostream>

namespace minirpc {

RpcClient::RpcClient(asio::io_service& io_service)
  : io_service_(io_service)
  , socket_(io_service) {
}

RpcClient::~RpcClient(void) {
}

void RpcClient::start(RpcHandler* handler, StartedHandler&& fn, const char* host, std::uint16_t port) {
  handler_ = handler;

  started_fn_ = std::move(fn);
  socket_.async_connect(asio::tcp::endpoint(asio::address::from_string(host), port),
      [this](const boost::system::error_code& ec) {
        if (!ec) {
          channel_ = std::make_shared<RpcChannel>(io_service_, std::move(socket_));
          service_ = std::make_shared<RpcService>(channel_.get(), handler_);
          channel_->set_service(service_.get());
          channel_->start();

          if (started_fn_)
            started_fn_();
        }
        else {
          socket_.close();
        }
      });
}

void RpcClient::close(void) {
  if (channel_)
    channel_->close();
}

}
