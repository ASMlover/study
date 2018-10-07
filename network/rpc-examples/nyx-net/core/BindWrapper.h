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

#include "MessageDefs.h"

namespace nyx {

class BindWrapper {
  BindWrapper(const BindWrapper&) = delete;
  BindWrapper& operator=(const BindWrapper&) = delete;
protected:
  ConnectedCallback connected_fn_{};
  NewConnectionCallback newconnection_fn_{};
  MessageCallback message_fn_{};

  BindWrapper(void) = default;
  ~BindWrapper(void) = default;
public:
  BindWrapper& set_connected_callback(const ConnectedCallback& fn) {
    connected_fn_ = fn;
    return *this;
  }

  BindWrapper& set_connected_callback(ConnectedCallback&& fn) {
    connected_fn_ = std::move(fn);
    return *this;
  }

  BindWrapper& set_newconnection_callback(const NewConnectionCallback& fn) {
    newconnection_fn_ = fn;
    return *this;
  }

  BindWrapper& set_newconnection_callback(NewConnectionCallback&& fn) {
    newconnection_fn_ = std::move(fn);
    return *this;
  }

  BindWrapper& set_message_callback(const MessageCallback& fn) {
    message_fn_ = fn;
    return *this;
  }

  BindWrapper& set_message_callback(MessageCallback& fn) {
    message_fn_ = std::move(fn);
    return *this;
  }
};

}
