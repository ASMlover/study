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

#include <map>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/lockfree/queue.hpp>
#include "utils/nyx_pyaux.h"
#include "utils/nyx_lockfree_container.h"
#include "crypter/nyx_crypter.h"
#include "rpc/nyx_rpc_service_helper.h"
#include "net/nyx_connection.h"
#include "nyx_base_service_factory.h"

namespace nyx {

class base_service;

using service_ptr = std::shared_ptr<base_service>;
using key_stack_type = lockfree::stack<crypter::key_crypter>;
using key_stack_node = lockfree::stack_node<crypter::key_crypter>;

class service_manager : private boost::noncopyable {
  using factory_ptr = std::shared_ptr<base_service_factory>;

  bool service_created_{};
  std::string keypath_;
  key_stack_type stack_;
  boost::lockfree::queue<nyx::rpc::service_call_helper*> requests_;
  factory_ptr factory_;

  service_manager(void) {}
  virtual ~service_manager(void) {}
public:
  static service_manager& instance(void) {
    static service_manager ins;
    return ins;
  }

  service_ptr get_service(service_type type, net::connection* conn);
  void push_request(rpc::service_call_helper* call_helper);
  std::size_t call_request(void);

  void set_service_factory(const factory_ptr& factory) {
    if (service_created_)
      abort();
    factory_ = factory;
  }

  factory_ptr get_service_factory(void) const {
    return factory_;
  }

  void set_keypath(const std::string& keypath) {
    keypath_ = keypath;
  }

  void set_key_crypter(key_stack_node* crypter) {
    stack_.push(crypter->value);
  }

  key_stack_node* get_key_crypter(void) {
    auto* crypter = stack_.pop();
    if (crypter == nullptr)
      crypter = new key_stack_node(new crypter::key_crypter(keypath_));
    return crypter;
  }
};

}
