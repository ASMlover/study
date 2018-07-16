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

#include <set>
#include <string>
#include <boost/noncopyable.hpp>
#include "../utils/nyx_pyaux.h"
#include "../utils/nyx_utils.h"

namespace nyx { namespace rpc {

class rpc_handler;

class rpc_handler_manager : private boost::noncopyable {
  using handler_set = std::set<rpc_handler*>;

  handler_set handlers_[static_cast<int>(service_type::type_end)];

  rpc_handler_manager(void) {}
  ~rpc_handler_manager(void) {}
public:
  static rpc_handler_manager& instance(void) {
    static rpc_handler_manager ins;
    return ins;
  }

  void regisrer_handler(rpc_handler* handler);
  void unregister_handler(rpc_handler* handler);
  std::size_t dispatch_rpc(service_type type,
      std::size_t filter, const std::string& method, const py::tuple& args);
  void broadcast_rpc(
      service_type type, const std::string& method, const py::tuple& args);

  std::size_t get_size(service_type type) const {
    if (type >= service_type::type_none && type < service_type::type_end) {
      auto i = static_cast<int>(type);
      return handlers_[i].size();
    }
    return 0;
  }
};

}}
