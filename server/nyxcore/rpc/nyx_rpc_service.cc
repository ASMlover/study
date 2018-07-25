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
#include "nyx_rpc_service.h"

namespace nyx { namespace rpc {

service::service(void)
  : delegate_(this)
  , channel_(this) {
}

service::~service(void) {
}

void service::on_guard_destroy(bool connected) {
  // TODO:
}

void service::set_service(base_service* service) {
  channel_.set_service(service);
  delegate_ = service;
}

void service::set_handler(const py::object& handler) {
  handler_ = handler;
}

void service::enable_compressor(bool enabled, unsigned char channel) {
  channel_.enable_compressor(enabled, channel);
}

void service::enable_encrypter(const std::string& key, unsigned char channel) {
  channel_.enable_encrypter(key, channel);
}

void service::set_recv_limit(std::size_t limit) {
  channel_.set_recv_limit(limit);
}

void service::set_wbits(int wbits) {
  channel_.set_wbits(wbits);
}

void service::set_memlevel(int memlevel) {
  channel_.set_memlevel(memlevel);
}

void service::traverse(const nyx::rpc::rpc_traverse_msg_ptr& msg) {
  channel_.set_traverse(msg);
}

void service::call_traverse(const nyx::rpc::rpc_traverse_msg_ptr& msg) {
  channel_.call_traverse(msg);
}

}}
