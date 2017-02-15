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

#include <functional>
#include <map>
#include <string>
#include "rpc_channel.h"

namespace minirpc {

typedef std::function<void (const std::string&)> MethodType;
class RpcHandler {
  minirpc::IRpcService::Stub* stub_{};

  std::map<std::string, MethodType> methods_;
public:
  RpcHandler(void) {}
  virtual ~RpcHandler(void) {}
  virtual void init_methods(void) = 0;

  void reg_method(const std::string& name, const MethodType& method) {
    methods_.insert(std::make_pair(name, method));
  }

  void call_method(const std::string& method_name, const std::string& arguments) {
    if (methods_.find(method_name) != methods_.end())
      methods_[method_name](arguments);
  }

  void set_stub(minirpc::IRpcService::Stub* stub) {
    stub_ = stub;
  }

  void call_method_proxy(const std::string& name, const std::string& arguments) {
    if (stub_) {
      minirpc::CallMessage message;
      message.set_method(name);
      message.set_arguments(arguments);
      stub_->call_method(nullptr, &message, nullptr, nullptr);
    }
  }
};

}

#define REG_HANDLER_METHOD(Class, Method)\
  reg_method(#Method, std::bind(&Class::Method, this, std::placeholders::_1))
