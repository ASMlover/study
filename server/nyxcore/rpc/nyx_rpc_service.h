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

#include <functional>
#include <memory>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "../utils/nyx_pyaux.h"
#include "../utils/nyx_callback_helper.h"
#include "../nyx_base_service.h"
#include "nyx_rpc_channel.h"
#include "nyx_rpc_handler.h"

namespace nyx { namespace rpc {

class service : public base_service, public destroy_guard {
protected:
  base_service* delegate_{};
  rpc_channel channel_;
  py::object handler_;
  bool delay_compress_{false};
  std::string delay_encrypt_key_;
public:
  service(void);
  virtual ~service(void);

  virtual void on_guard_destroy(bool connected) override;
  virtual void set_service(base_service* service) override;
  virtual void set_handler(const py::object& handler) override;
  virtual void enable_compressor(bool enabled, unsigned char channel) override;
  virtual void enable_encrypter(const std::string& key, unsigned char channel) override;
  virtual void set_recv_limit(std::size_t limit) override;
  virtual void set_wbits(int wbits) override;
  virtual void set_memlevel(int memlevel) override;
  virtual void traverse(const nyx::rpc::rpc_traverse_msg_ptr& msg) override;
  virtual void call_traverse(const nyx::rpc::rpc_traverse_msg_ptr& msg) override;

  virtual void set_delay_encrypt_key(const std::string& key) override {
    delay_encrypt_key_ = key;
  }

  virtual void set_delay_enable_compress(void) override {
    delay_compress_ = true;
  }
};

}}
