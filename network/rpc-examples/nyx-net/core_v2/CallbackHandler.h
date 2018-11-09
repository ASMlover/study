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

namespace nyx {

class BaseSession;
using SessionPtr = std::shared_ptr<BaseSession>;

using ConnectedCallback = std::function<void (const SessionPtr&)>;
using ConnectErrorCallback = std::function<void (const SessionPtr&)>;
using NewConnectionCallback = std::function<void (const SessionPtr&)>;
using MessageCallback = std::function<void (const SessionPtr&, const std::string&)>;
using DisconnetedCallback = std::function<void (const SessionPtr&)>;

struct CallbackHandler {
  ConnectedCallback on_connected_{};
  ConnectErrorCallback on_connect_error_{};
  NewConnectionCallback on_new_connection_{};
  MessageCallback on_message_{};
  DisconnetedCallback on_disconnected_{};

  void set_connected_callback(const ConnectedCallback& fn) {
    on_connected_ = fn;
  }

  void set_connected_callback(ConnectedCallback&& fn) {
    on_connected_ = std::move(fn);
  }

  void set_connect_error_callback(const ConnectErrorCallback& fn) {
    on_connect_error_ = fn;
  }

  void set_connect_error_callback(ConnectErrorCallback&& fn) {
    on_connect_error_ = std::move(fn);
  }

  void set_new_connection_callback(const NewConnectionCallback& fn) {
    on_new_connection_ = fn;
  }

  void set_new_connection_callback(NewConnectionCallback&& fn) {
    on_new_connection_ = std::move(fn);
  }

  void set_message_callback(const MessageCallback& fn) {
    on_message_ = fn;
  }

  void set_message_callback(MessageCallback&& fn) {
    on_message_ = std::move(fn);
  }

  void set_disconnected_callback(const DisconnetedCallback& fn) {
    on_disconnected_ = fn;
  }

  void set_disconnected_callback(DisconnetedCallback&& fn) {
    on_disconnected_ = std::move(fn);
  }
};

}
