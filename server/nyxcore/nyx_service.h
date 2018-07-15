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

#include <boost/python.hpp>
#include <boost/noncopyable.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include "rpc/nyx_rpc_common.h"

namespace nyx {

namespace py = ::boost::python;
namespace pb = ::google::protobuf;

class base_service : private boost::noncopyable {
public:
  virtual ~base_service(void) {}
  virtual const pb::ServiceDescriptor* get_descriptor(void) = 0;
  virtual const pb::Message& get_request_prototype(
      const pb::MethodDescriptor* method) const = 0;
  virtual const pb::Message& get_response_prototype(
      const pb::MethodDescriptor* method) const = 0;
  virtual void call_method(
      const pb::MethodDescriptor* method,
      pb::RpcController* controller,
      const pb::Message* request,
      pb::Message* response,
      pb::Closure* done) = 0;
  virtual void do_call_method(
      const pb::MethodDescriptor* method, const pb::Message* request) = 0;
  virtual const pb::ServiceDescriptor* get_stub_descriptor(void) = 0;
  virtual const pb::Message& get_stub_request_prototype(
      const pb::MethodDescriptor* method) = 0;
  virtual const pb::Message& get_stub_response_prototype(
      const pb::MethodDescriptor* method) = 0;
  virtual void call_rpc_method(
      const pb::MethodDescriptor* method,
      pb::RpcController* controller,
      const pb::Message* request,
      pb::Message* response,
      pb::Closure* done) = 0;
  virtual void dispatch_rpc(const std::string& method, const py::tuple& args) = 0;

  virtual void on_guard_destroy(bool connected) = 0;
  virtual void set_service(base_service* service) = 0;
  virtual void set_handler(const py::object& handler) = 0;
  virtual void enable_compressor(bool enabled, unsigned char channel) = 0;
  virtual void enable_encrypter(const std::string& key, unsigned char channel) = 0;
  virtual void set_delay_encrypt_key(const std::string& key) = 0;
  virtual void set_delay_enable_compress(void) = 0;
  virtual void set_recv_limit(std::size_t limit) = 0;
  virtual void set_wbits(int wbits) = 0;
  virtual void set_memlevel(int memlevel) = 0;
  virtual void traverse(const nyx::rpc::rpc_traverse_msg_ptr& msg) = 0;
  virtual void call_traverse(const nyx::rpc::rpc_traverse_msg_ptr& msg) = 0;
  virtual bool handle_data(const char* data, std::size_t size) = 0;
  virtual bool handle_data(const char* data,
      std::size_t size, bool reliable, unsigned char channel) = 0;
  virtual void async_write(const nyx::rpc::writbuf_ptr& buf) = 0;
  virtual void async_write(const nyx::rpc::writbuf_ptr& buf,
      bool reliable, unsigned char channel) = 0;
  virtual void disconnect(void) = 0;
  virtual const std::string get_remote_addr(void) const = 0;
  virtual std::uint16_t get_remote_port(void) const = 0;
};

}
