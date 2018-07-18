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
#include <boost/noncopyable.hpp>
#include "../utils/nyx_pyaux.h"
#include "../utils/nyx_callback_helper.h"
#include "nyx_rpc_channel.h"
#include "nyx_rpc_handler.h"

namespace nyx { namespace rpc {

using channel_callback = std::function<void (void)>;

class service_call_helper : private boost::noncopyable {
public:
  service_call_helper(void);
  virtual ~service_call_helper(void);
  virtual void call_method(void) = 0;
};

class rpc_service_call_helper : public service_call_helper {
  base_service* service_{};
  nyx::callback_guard_ptr guard_;
  const pb::MethodDescriptor* method_{};
  const pb::Message* request_{};
public:
  rpc_service_call_helper(
      base_service* service,
      const nyx::callback_guard_ptr& guard,
      const pb::MethodDescriptor* method,
      const pb::Message* request);
  virtual ~rpc_service_call_helper(void);
  virtual void call_method(void) override;
};

class callback_service_call_helper : public service_call_helper {
  channel_callback callback_{};
public:
  callback_service_call_helper(const channel_callback& callback);
  callback_service_call_helper(channel_callback&& callback);
  virtual ~callback_service_call_helper(void);
  virtual void call_method(void) override;
};

}}
