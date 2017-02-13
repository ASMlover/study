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

#include <atomic>
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

namespace minirpc {

using ::boost::asio::ip::tcp;
namespace gpb = ::google::protobuf;

class RpcMessage;

class RpcChannel : public gpb::RpcChannel, public std::enable_shared_from_this<RpcChannel> {
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  std::vector<char> buffer_;

  struct OutstandingCall {
    gpb::Message* response;
    gpb::Closure* done;
  };
  std::atomic<std::int64_t> id_;
  mutable std::mutex mutex_;
  std::map<std::int64_t, OutstandingCall> outstandings_;
  std::map<std::string, gpb::Service*> services_;

  void do_write(const RpcMessage& message);
  void do_read_header(void);
  void do_read_body(int len);
  void handle_message(const RpcMessage& message);
  void done_callback(gpb::Message* response, std::int64_t id);
public:
  explicit RpcChannel(boost::asio::io_service& io_service, tcp::socket&& socket);
  ~RpcChannel(void);

  virtual void CallMethod(const gpb::MethodDescriptor* method,
      gpb::RpcController* controller,
      const gpb::Message* request,
      gpb::Message* response,
      gpb::Closure* done) override;

  void start(void);
  void close(void);

  void set_services(const std::map<std::string, gpb::Service*>& services) {
    services_ = services;
  }
};

}
