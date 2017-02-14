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
#include "rpc.pb.h"
#include "codec.h"
#include "rpc_channel.h"

namespace minirpc {

RpcChannel::RpcChannel(boost::asio::io_service& io_service, tcp::socket&& socket)
  : io_service_(io_service)
  , socket_(std::move(socket)) {
}

RpcChannel::~RpcChannel(void) {
}

void RpcChannel::CallMethod(const gpb::MethodDescriptor* method,
    gpb::RpcController* /*controller*/,
    const gpb::Message* request,
    gpb::Message* response,
    gpb::Closure* done) {
  std::int64_t id = ++id_;

  minirpc::RpcMessage message;
  message.set_type(MT_REQUEST);
  message.set_id(id);
  message.set_service(method->service()->name());
  message.set_method(method->name());
  message.set_request(request->SerializeAsString());

  {
    OutstandingCall out{response, done};
    std::unique_lock<std::mutex> guard(mutex_);
    outstandings_[id] = out;
  }

  do_write(message);
}

void RpcChannel::start(void) {
  do_read_header();
}

void RpcChannel::close(void) {
  auto self(shared_from_this());
  io_service_.post([this, self] { socket_.close(); });
}

void RpcChannel::do_write(const minirpc::RpcMessage& message) {
  std::vector<char> buf;
  minirpc::encode(message, buf);

  auto self(shared_from_this());
  boost::asio::async_write(socket_, boost::asio::buffer(buf),
      [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
        if (ec) {
          std::cout << "RpcChannel::do_write - write message failed ...." << std::endl;
          socket_.close();
        }
      });
}

void RpcChannel::do_read_header(void) {
  auto self(shared_from_this());
  buffer_.resize(4);
  boost::asio::async_read(socket_, boost::asio::buffer(buffer_),
      [this, self](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && n == 4) {
          int len = 0;
          std::memcpy(&len, buffer_.data(), 4);
          do_read_body(len);
        }
        else {
          socket_.close();
        }
      });
}

void RpcChannel::do_read_body(int len) {
  auto self(shared_from_this());
  buffer_.resize(len);
  boost::asio::async_read(socket_, boost::asio::buffer(buffer_),
      [this, self, len](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && static_cast<std::size_t>(len) == n) {
          minirpc::RpcMessage message;
          minirpc::ParseError r = minirpc::decode(buffer_.data(), len, message);
          if (r == minirpc::ParseError::SUCCESS)
            handle_message(message);

          do_read_header();
        }
        else {
          socket_.close();
        }
      });
}

void RpcChannel::handle_message(const RpcMessage& message) {
  if (message.type() == minirpc::MessageType::MT_REQUEST) {
    auto it = services_.find(message.service());
    if (it != services_.end()) {
      auto* service = it->second;
      const auto* desc = service->GetDescriptor();
      const auto* method = desc->FindMethodByName(message.method());
      if (method) {
        auto* request = service->GetRequestPrototype(method).New();
        request->ParseFromString(message.request());
        auto* response = service->GetResponsePrototype(method).New();
        std::int64_t id = message.id();
        service->CallMethod(method, nullptr, request, response,
            gpb::NewCallback(this, &RpcChannel::done_callback, response, id));
        delete request;
      }
    }
  }
  else if (message.type() == minirpc::MessageType::MT_RESPONSE) {
    std::int64_t id = message.id();
    OutstandingCall out{};

    {
      std::unique_lock<std::mutex> guard(mutex_);
      auto it = outstandings_.find(id);
      if (it != outstandings_.end()) {
        out = it->second;
        outstandings_.erase(it);
      }
    }

    if (out.response) {
      out.response->ParseFromString(message.response());
      if (out.done)
        out.done->Run();
      delete out.response;
    }
  }
}

void RpcChannel::done_callback(gpb::Message* response, std::int64_t id) {
  minirpc::RpcMessage message;
  message.set_type(MT_RESPONSE);
  message.set_id(id);
  message.set_response(response->SerializeAsString());
  do_write(message);

  delete response;
}

}
