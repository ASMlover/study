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

#include <functional>
#include <memory>

namespace nyx::net {

class BaseSession;
using SessionPtr = std::shared_ptr<BaseSession>;

using ResolveErrorCallback = std::function<void (const SessionPtr&)>;
using ConnectedCallback = std::function<void (const SessionPtr&)>;
using ConnectErrorCallback = std::function<void (const SessionPtr&)>;
using NewConnectionCallback = std::function<void (const SessionPtr&)>;
using MessageCallback = std::function<void (const SessionPtr&, const std::string&)>;
using DisconnetedCallback = std::function<void (const SessionPtr&)>;

struct CallbackHandler {
  ResolveErrorCallback on_resolve_error{};
  ConnectedCallback on_connected{};
  ConnectErrorCallback on_connect_error{};
  NewConnectionCallback on_new_connection{};
  MessageCallback on_message{};
  DisconnetedCallback on_disconnected{};

  auto get_shared_callback(void) const {
    return std::make_shared<CallbackHandler>(*this);
  }

  CallbackHandler& set_resolve_error_callback(const ResolveErrorCallback& fn) {
    on_resolve_error = fn;
    return *this;
  }

  CallbackHandler& set_resolve_error_callback(ResolveErrorCallback&& fn) {
    on_resolve_error = std::move(fn);
    return *this;
  }

  CallbackHandler& set_connected_callback(const ConnectedCallback& fn) {
    on_connected = fn;
    return *this;
  }

  CallbackHandler& set_connected_callback(ConnectedCallback&& fn) {
    on_connected = std::move(fn);
    return *this;
  }

  CallbackHandler& set_connect_error_callback(const ConnectErrorCallback& fn) {
    on_connect_error = fn;
    return *this;
  }

  CallbackHandler& set_connect_error_callback(ConnectErrorCallback&& fn) {
    on_connect_error = std::move(fn);
    return *this;
  }

  CallbackHandler& set_new_connection_callback(
      const NewConnectionCallback& fn) {
    on_new_connection = fn;
    return *this;
  }

  CallbackHandler& set_new_connection_callback(NewConnectionCallback&& fn) {
    on_new_connection = std::move(fn);
    return *this;
  }

  CallbackHandler& set_message_callback(const MessageCallback& fn) {
    on_message = fn;
    return *this;
  }

  CallbackHandler& set_message_callback(MessageCallback&& fn) {
    on_message = std::move(fn);
    return *this;
  }

  CallbackHandler& set_disconnected_callback(const DisconnetedCallback& fn) {
    on_disconnected = fn;
    return *this;
  }

  CallbackHandler& set_disconnected_callback(DisconnetedCallback&& fn) {
    on_disconnected = std::move(fn);
    return *this;
  }
};

}
