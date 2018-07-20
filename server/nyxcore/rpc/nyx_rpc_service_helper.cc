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
#include "../nyx_service.h"
#include "nyx_rpc_service_helper.h"

namespace nyx { namespace rpc {

rpc_service_call_helper::rpc_service_call_helper(
    base_service* service,
    const nyx::callback_guard_ptr& guard,
    const pb::MethodDescriptor* method,
    const pb::Message* request)
  : service_(service)
  , guard_(guard)
  , method_(method)
  , request_(request) {
}

rpc_service_call_helper::~rpc_service_call_helper(void) {
  if (request_ != nullptr)
    delete request_;
}

void rpc_service_call_helper::call_method(void) {
  if (guard_->is_calling())
    service_->do_call_method(method_, request_);
}

callback_service_call_helper::callback_service_call_helper(
    const channel_callback& callback)
  : callback_(callback) {
}

callback_service_call_helper::callback_service_call_helper(
    channel_callback&& callback)
  : callback_(std::move(callback)) {
}

callback_service_call_helper::~callback_service_call_helper(void) {
}

void callback_service_call_helper::call_method(void) {
  if (callback_)
    callback_();
}

}}
