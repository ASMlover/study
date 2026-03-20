// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <unordered_map>
#include "Common.hh"
#include "Json.hh"

namespace ms {

class LspServer : private UnCopyable {
  bool running_{true};
  bool initialized_{false};
  std::unordered_map<str_t, str_t> documents_;

  // Transport
  str_t read_message() noexcept;
  void send_message(const json::Value& msg) noexcept;
  void send_response(const json::Value& id, json::Value result) noexcept;
  void send_error(const json::Value& id, int code, strv_t message) noexcept;
  void send_notification(strv_t method, json::Value params) noexcept;

  // Dispatch
  void dispatch(const json::Value& msg) noexcept;

  // Request handlers
  void handle_initialize(const json::Value& id, const json::Value& params) noexcept;
  void handle_shutdown(const json::Value& id) noexcept;
  void handle_semantic_tokens_full(const json::Value& id, const json::Value& params) noexcept;
  void handle_document_symbol(const json::Value& id, const json::Value& params) noexcept;

  // Notification handlers
  void handle_initialized() noexcept;
  void handle_exit() noexcept;
  void handle_did_open(const json::Value& params) noexcept;
  void handle_did_change(const json::Value& params) noexcept;
  void handle_did_close(const json::Value& params) noexcept;

  // Features
  void publish_diagnostics(const str_t& uri) noexcept;
  json::Value compute_semantic_tokens(const str_t& source) noexcept;
  json::Value compute_document_symbols(const str_t& source) noexcept;

  // Helpers
  str_t uri_to_path(const str_t& uri) noexcept;

public:
  void run() noexcept;
};

} // namespace ms
