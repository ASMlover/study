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
#include "nyx_rpc_handler.h"

namespace nyx { namespace rpc {

void rpc_handler::set_service(base_service* service) {
}

void rpc_handler::register_to_manager(void) {
}

void rpc_handler::traverse(const rpc_traverse_msg_ptr& msg) {
}

void rpc_handler::call_traverse(const rpc_traverse_msg_ptr& msg) {
}

bool rpc_handler::dispatch_rpc(
    const std::string& method, const py::tuple& args) {
  return false;
}

bool rpc_handler_wrapper::dispatch_rpc(
    const std::string& method, const py::tuple& args) {
  return false;
}

void rpc_handler_wrapper::delay_enable_compress(void) {
}

void rpc_handler_wrapper::delay_enable_encrypt(const std::string& key) {
}

void rpc_handler_wrapper::disconnect(void) {
}

void rpc_handler_wrapper::enable_compressor(
    bool enabled, unsigned char channel) {
}

void rpc_handler_wrapper::enable_encrypter(
    const std::string& key, unsigned char channel) {
}

void rpc_handler_wrapper::set_wbits(int wbits) {
}

void rpc_handler_wrapper::set_memlevel(int memlevel) {
}

py::tuple rpc_handler_wrapper::getpeername(void) const {
  return py::tuple();
}

void rpc_handler_wrapper::set_userdata(
    const std::string& key, PyObject* value) {
}

void rpc_handler_wrapper::unset_userdata(const std::string& key) {
}

void rpc_handler_wrapper::clear_userdatas(void) {
}

}}
