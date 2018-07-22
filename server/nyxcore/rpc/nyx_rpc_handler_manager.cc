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
#include "../filter/nyx_filter_manager.h"
#include "nyx_rpc_handler.h"
#include "nyx_rpc_handler_manager.h"

namespace nyx { namespace rpc {

void rpc_handler_manager::regisrer_handler(rpc_handler* handler) {
  if (handler != nullptr) {
    auto i = static_cast<int>(handler->get_type());
    handlers_[i].insert(handler);
  }
}

void rpc_handler_manager::unregister_handler(rpc_handler* handler) {
  if (handler != nullptr) {
    auto i = static_cast<int>(handler->get_type());
    handlers_[i].erase(handler);
  }
}

std::size_t rpc_handler_manager::dispatch_rpc(service_type type,
    std::size_t filter, const std::string& method, const py::tuple& args) {
  if (type < service_type::type_none || type >= service_type::type_end)
    return 0;

  auto& hs = handlers_[static_cast<int>(type)];
  std::size_t counter{};
  if (filter > 0) {
    auto fp = nyx::filter::filter_manager::instance().get_filter(filter);
    if (!fp)
      return 0;
    auto msg = std::make_shared<rpc_traverse_msg>();
    for (auto* h : hs) {
      if (!h->filter(fp))
        continue;

      if (msg->empty()) {
        h->traverse(msg);
        h->dispatch_rpc(method, args);
      }
      else {
        h->call_traverse(msg);
      }
      ++counter;
    }
  }
  else {
    auto msg = std::make_shared<rpc_traverse_msg>();
    for (auto* h : hs) {
      if (msg->empty()) {
        h->traverse(msg);
        h->dispatch_rpc(method, args);
      }
      else {
        h->call_traverse(msg);
      }
    }
    counter = hs.size();
  }

  return counter;
}

void rpc_handler_manager::broadcast_rpc(
    service_type type, const std::string& method, const py::tuple& args) {
  if (type < service_type::type_none || type >= service_type::type_end)
    return;

  auto& hs = handlers_[static_cast<int>(type)];
  auto msg = std::make_shared<rpc_traverse_msg>();
  for (auto* h : hs) {
    if (msg->empty()) {
      h->traverse(msg);
      h->dispatch_rpc(method, args);
    }
    else {
      h->call_traverse(msg);
    }
  }
}

}}
