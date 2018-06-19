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

#include <string>
#include <google/protobuf/service.h>

namespace pb = ::google::protobuf;

namespace nyx { namespace rpc {

class rpc_controller : public pb::RpcController {
  static constexpr unsigned int kChannelCount = 2;
  static constexpr unsigned char kDefaultChannelId = 0;

  bool reliable_{};
  unsigned char channel_{};
public:
  rpc_controller(void)
    : reliable_(true)
    , channel_(kDefaultChannelId) {
  }

  virtual ~rpc_controller(void) {}

  virtual void Reset(void) override {}
  virtual bool Failed(void) const override { return false; }
  virtual std::string ErrorText(void) const override { return ""; }
  virtual void StartCancel(void) override {}
  virtual void SetFailed(const std::string& reason) override {}
  virtual bool IsCanceled(void) const override { return false; }
  virtual void NotifyOnCancel(pb::Closure* callback) override {}

  void set_reliable(bool r) {
    reliable_ = r;
  }

  bool get_reliable(void) const {
    return reliable_;
  }

  void set_channel(unsigned char c) {
    channel_ = c;
  }

  unsigned char get_channel(void) const {
    return channel_;
  }
};

}}
