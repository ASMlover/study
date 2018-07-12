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
#include "nyx_rpc_channel.h"

namespace nyx { namespace rpc {

rpc_channel::rpc_channel(base_service* service)
  : wbits_(12)
  , memlevel_(5)
  , service_(service) {
}

rpc_channel::~rpc_channel(void) {
}

bool rpc_channel::on_request(unsigned char channel) {
  return false;
}

void rpc_channel::CallMethod(
    const pb::MethodDescriptor* method,
    pb::RpcController* controller,
    const pb::Message* request,
    pb::Message* response,
    pb::Closure* done) {
}

bool rpc_channel::handle_data(
    const char* data, std::size_t size, bool reliable, unsigned char channel) {
  return false;
}

void rpc_channel::set_recv_limit(std::size_t limit) {
}

void rpc_channel::enable_compressor(bool enabled, unsigned char channel) {
}

void rpc_channel::enable_encrypter(
    const std::string& key, unsigned char channel) {
}

void rpc_channel::call_traverse(const rpc_traverse_msg_ptr& msg) {
}

}}
