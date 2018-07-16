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
#include "../utils/nyx_utils.h"
#include "../utils/nyx_pyaux.h"
#include "../filter/nyx_filter.h"
#include "nyx_rpc_common.h"
#include "nyx_rpc_controller.h"
#include "nyx_rpc_handler_userdata.h"

namespace nyx {

class base_service;

namespace rpc {

class rpc_handler : private boost::noncopyable {
protected:
  service_type service_type_{};
  base_service* service_{};
  rpc_handler_userdata userdata_;
public:
  explicit rpc_handler(service_type type) : service_type_(type) {}
  virtual ~rpc_handler(void) {}

  void set_service(base_service* service);
  void register_to_manager(void);
  void traverse(const rpc_traverse_msg_ptr& msg);
  void call_traverse(const rpc_traverse_msg_ptr& msg);
  virtual bool dispatch_rpc(const std::string& method, const py::tuple& args);

  base_service* get_service(void) const {
    return service_;
  }

  service_type get_type(void) const {
    return service_type_;
  }

  bool filter(const filter::filter_ptr& filter) {
    return userdata_.done(filter);
  }
};

class rpc_handler_wrapper : public rpc_handler {
  PyObject* self_{};
public:
  rpc_handler_wrapper(PyObject* self, service_type type)
    : rpc_handler(type), self_(self) {
  }
  virtual ~rpc_handler_wrapper(void) {}

  virtual bool dispatch_rpc(
      const std::string& method, const py::tuple& args) override;

  void delay_enable_compress(void);
  void delay_enable_encrypt(const std::string& key);
  void disconnect(void);
  void enable_compressor(
      bool enabled, unsigned char channel = kDefaultChannelId);
  void enable_encrypter(
      const std::string& key, unsigned char channel = kDefaultChannelId);
  void set_wbits(int wbits);
  void set_memlevel(int memlevel);
  py::tuple getpeername(void) const;
  void set_userdata(const std::string& key, PyObject* value);
  void unset_userdata(const std::string& key);
  void clear_userdatas(void);
};

}}
