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
#include <unordered_map>
#include <asio.hpp>
#include <asio/system_timer.hpp>
#include "Types.h"
#include "Callbacks.h"

namespace KcpNet {

using asio::ip::udp;

class Server : private UnCopyable {
  static constexpr std::size_t kBufferSize = 32 << 10;

  bool stopped_{};
  udp::socket socket_;
  asio::system_timer timer_;
  udp::endpoint sender_ep_;
  std::vector<char> readbuff_;
  std::unordered_map<kcp_conv_t, SessionPtr> sessions_;
  ConnectionCallback connection_fn_{};
  MessageCallback message_fn_{};

  void do_read(void);
  void do_timer(void);
  kcp_conv_t gen_conv(void) const;
  void write_udp(const char* buf, std::size_t len, const udp::endpoint& ep);
  void write_udp(const std::string& buf, const udp::endpoint& ep);
public:
  Server(asio::io_service& io_service, std::uint16_t port);
  ~Server(void);

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
