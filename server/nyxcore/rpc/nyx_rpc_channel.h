// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <google/protobuf/service.h>
#include "nyx_rpc_common.h"
#include "nyx_rpc_controller.h"
#include "nyx_rpc_request_parser.h"

namespace nyx {

class base_service;

namespace rpc {

namespace pb = ::google::protobuf;

class rpc_channel;
class rpc_converter;

using method_index_type = unsigned short;
using rpc_channel_ptr = std::shared_ptr<rpc_channel>;
using rpc_converter_ptr = std::shared_ptr<rpc_converter>;

class rpc_channel : public pb::RpcChannel, private boost::noncopyable {
  int wbits_{};
  int memlevel_{};
  base_service* service_{};
  rpc_request requests_[kChannelCount];
  rpc_request_parser request_parsers_[kChannelCount];
  rpc_converter_ptr converters_[kChannelCount];
  rpc_traverse_msg_ptr traverse_;

  bool on_request(unsigned char channel);
public:
  explicit rpc_channel(base_service* service);
  virtual ~rpc_channel(void);

  virtual void CallMethod(
      const pb::MethodDescriptor* method,
      pb::RpcController* controller,
      const pb::Message* request,
      pb::Message* response,
      pb::Closure* done) override;

  void set_service(base_service* service);
  void set_recv_limit(std::size_t limit);

  bool handle_data(
      const char* data, std::size_t size, bool reliable, unsigned char channel);
  void enable_compressor(bool enabled, unsigned char channel);
  void enable_encrypter(const std::string& key, unsigned char channel);
  void call_traverse(const rpc_traverse_msg_ptr& msg);

  bool handle_data(const char* data, std::size_t size) {
    return handle_data(data, size, true, kDefaultChannelId);
  }

  void set_wbits(int wbits) {
    wbits_ = wbits;
  }

  void set_memlevel(int memlevel) {
    memlevel_ = memlevel;
  }

  void set_traverse(const rpc_traverse_msg_ptr& msg) {
    traverse_ = msg;
  }
};

}}
