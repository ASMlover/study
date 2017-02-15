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
#include "rpc_channel.h"

namespace minirpc {

RpcChannel::RpcChannel(asio::io_service& io_service, asio::tcp::socket&& socket)
  : io_service_(io_service)
  , socket_(std::move(socket)) {
}

RpcChannel::~RpcChannel(void) {
}

void RpcChannel::CallMethod(const gpb::MethodDescriptor* method, gpb::RpcController* /*controller*/,
    const gpb::Message* request, gpb::Message* response, gpb::Closure* done) {
  std::int64_t id = ++id_;

  minirpc::RpcMessage message;
  message.set_type(TYPE_REQUEST);
  message.set_id(id);
  message.set_service(method->service()->name());
  message.set_method(method->name());
  message.set_request(request->SerializeAsString());

  {
    OutstandingCall out{response, done};
    std::unique_lock<std::mutex> guard(mutex_);
    outstandings_.insert(std::make_pair(id, out));
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

void RpcChannel::do_read_header(void) {
  buffer_.resize(4);
  auto self(shared_from_this());
  asio::async_read(socket_, asio::buffer(buffer_),
      [this, self](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && n == 4) {
          std::size_t len = 0;
          std::memcpy(&len, buffer_.data(), 4);
          do_read_body(len);
        }
        else {
          socket_.close();
        }
      });
}

void RpcChannel::do_read_body(std::size_t len) {
  buffer_.resize(len);
  auto self(shared_from_this());
  asio::async_read(socket_, asio::buffer(buffer_),
      [this, self, len](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && len == n) {
          minirpc::RpcMessage message;
          if (decode(buffer_, message))
            handle_message(message);

          do_read_header();
        }
        else {
          socket_.close();
        }
      });
}

void RpcChannel::do_write(const RpcMessage& message) {
  std::vector<char> buf;
  encode(message, buf);

  auto self(shared_from_this());
  asio::async_write(socket_, asio::buffer(buf),
      [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
        if (ec) {
          std::cerr << "RpcChannel::do_write - write RpcMessage failed ..." << std::endl;
          socket_.close();
        }
      });
}

void RpcChannel::handle_message(const RpcMessage& message) {
  if (message.type() == TYPE_REQUEST) {
    if (service_) {
      const auto* desc = service_->GetDescriptor();
      const auto* method = desc->FindMethodByName(message.method());
      if (method) {
        auto* request = service_->GetRequestPrototype(method).New();
        request->ParseFromString(message.request());
        service_->CallMethod(method, nullptr, request, nullptr, nullptr);
        // TODO: solve response

        delete request;
      }
    }
  }
  else if (message.type() == TYPE_RESPONSE) {
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
  message.set_type(TYPE_RESPONSE);
  message.set_id(id);
  message.set_response(response->SerializeAsString());
  do_write(message);

  delete response;
}

void RpcChannel::encode(const RpcMessage& msg, std::vector<char>& buf) {
  const int bytes = msg.ByteSize();
  const int len = bytes + 4; // RPC0
  const int ntotal = len + 4;

  buf.resize(ntotal);
  std::memcpy(&buf[0], &len, sizeof(len));
  std::memcpy(&buf[4], "RPC0", 4);
  msg.SerializeWithCachedSizesToArray((std::uint8_t*)&buf[8]);
}

bool RpcChannel::decode(const std::vector<char>& buf, RpcMessage& msg) {
  if (std::memcmp(buf.data(), "RPC0", 4) == 0) {
    if (msg.ParseFromArray(&buf[4], buf.size() - 4))
      return true;
  }

  return false;
}

}
