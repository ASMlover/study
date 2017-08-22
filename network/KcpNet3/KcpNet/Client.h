// Copyright (c) 2017 ASMlover. All rights reserved.
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

#include <vector>
#include <asio.hpp>
#include <asio/system_timer.hpp>
#include "Types.h"
#include "Callbacks.h"

namespace KcpNet {

using asio::ip::udp;

class Client : private UnCopyable {
  static constexpr std::size_t kBufferSize = 32 << 10;
  static constexpr std::uint64_t kConnectTimeout = 5000;

  bool stopped_{};
  bool connecting_{};
  bool connected_{};
  std::uint64_t connect_begtime_{};
  udp::socket socket_;
  asio::system_timer timer_;
  std::vector<char> readbuff_;
  SessionPtr session_{};
  ErrorCallback error_fn_{}; // connect failed
  ConnectionCallback connection_fn_{};
  MessageCallback message_fn_{};

  void do_read_connection(void);
  void do_write_connecttion(void);
  void do_read(void);
  void do_timer(void);
  void write_udp(const char* buf, std::size_t len);
  void write_udp(const std::string& buf);
public:
  Client(asio::io_context& io_context, std::uint16_t port);
  ~Client(void);
  void connect(const std::string& remote_ip, std::uint16_t remote_port);

  void bind_error_functor(const ErrorCallback& fn) {
    error_fn_ = fn;
  }

  void bind_error_functor(ErrorCallback&& fn) {
    error_fn_ = std::move(fn);
  }

  void bind_connecttion_functor(const ConnectionCallback& fn) {
    connection_fn_ = fn;
  }

  void bind_connecttion_functor(ConnectionCallback&& fn) {
    connection_fn_ = std::move(fn);
  }

  void bind_message_functor(const MessageCallback& fn) {
    message_fn_ = fn;
  }

  void bind_message_functor(MessageCallback&& fn) {
    message_fn_ = std::move(fn);
  }
};

}
