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
#include "../nyx_base_service.h"
#include "nyx_rpc_handler_manager.h"
#include "nyx_rpc_handler.h"

namespace nyx { namespace rpc {

void rpc_handler::set_service(base_service* service) {
  service_ = service;
  if (service_ == nullptr)
    rpc_handler_manager::instance().unregister_handler(this);
  else if (service_type_ != service_type::type_gate)
    rpc_handler_manager::instance().regisrer_handler(this);
}

void rpc_handler::register_to_manager(void) {
  if (service_ != nullptr && service_type_ == service_type::type_gate)
    rpc_handler_manager::instance().unregister_handler(this);
}

void rpc_handler::traverse(const rpc_traverse_msg_ptr& msg) {
  if (service_ != nullptr)
    service_->traverse(msg);
}

void rpc_handler::call_traverse(const rpc_traverse_msg_ptr& msg) {
  if (service_ != nullptr)
    service_->call_traverse(msg);
}

bool rpc_handler_wrapper::dispatch_rpc(
    const std::string& method, const py::tuple& args) {
  return service_ != nullptr ? service_->dispatch_rpc(method, args) : false;
}

void rpc_handler_wrapper::delay_enable_compress(void) {
  if (service_ != nullptr)
    service_->set_delay_enable_compress();
}

void rpc_handler_wrapper::delay_enable_encrypt(const std::string& key) {
  if (service_ != nullptr)
    service_->set_delay_encrypt_key(key);
}

void rpc_handler_wrapper::disconnect(void) {
    service_->disconnect();
}

void rpc_handler_wrapper::enable_compressor(
    bool enabled, unsigned char channel) {
  if (service_ != nullptr)
    service_->enable_compressor(enabled, channel);
}

void rpc_handler_wrapper::enable_encrypter(
    const std::string& key, unsigned char channel) {
  if (service_ != nullptr)
    service_->enable_encrypter(key, channel);
}

void rpc_handler_wrapper::set_wbits(int wbits) {
  if (service_ != nullptr)
    service_->set_wbits(wbits);
}

void rpc_handler_wrapper::set_memlevel(int memlevel) {
  if (service_ != nullptr)
    service_->set_memlevel(memlevel);
}

py::tuple rpc_handler_wrapper::getpeername(void) const {
  if (service_ != nullptr) {
    return py::make_tuple(
        service_->get_remote_addr(), service_->get_remote_port());
  }
  else {
    return py::make_tuple("", 0);
  }
}

void rpc_handler_wrapper::set_userdata(
    const std::string& key, PyObject* value) {
  if (PyInt_Check(value)) {
    auto raw_value = PyInt_AsSsize_t(value);
    if (raw_value == -1 && PyErr_Occurred())
      py::throw_error_already_set();
    userdata_.set_userdata<int>(key, raw_value);
  }
  else if (PyLong_Check(value)) {
    auto raw_value = static_cast<int>(PyLong_AsSsize_t(value));
    if (raw_value == -1L && PyErr_Occurred())
      py::throw_error_already_set();
    userdata_.set_userdata<int>(key, raw_value);
  }
  else if (PyFloat_Check(value)) {
    auto raw_value = static_cast<float>(PyFloat_AsDouble(value));
    if (raw_value == -1.0f && PyErr_Occurred())
      py::throw_error_already_set();
    userdata_.set_userdata<float>(key, raw_value);
  }
  else if (PyString_Check(value)) {
    auto* raw_value = PyString_AsString(value);
    if (raw_value == nullptr && PyErr_Occurred())
      py::throw_error_already_set();
    userdata_.set_userdata<const std::string&>(key, raw_value);
  }
  else {
    PyErr_SetString(PyExc_TypeError, "value should be int,long,float or str");
    py::throw_error_already_set();
  }
}

void rpc_handler_wrapper::unset_userdata(const std::string& key) {
  userdata_.remove_userdata(key);
}

void rpc_handler_wrapper::clear_userdatas(void) {
  userdata_.clear_userdata();
}

}}
