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
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include "KcpTypes.h"
#include "Callbacks.h"

namespace KcpNet {

using boost::asio::ip::udp;

class KcpServer : private boost::noncopyable {
  static constexpr std::size_t kBufferSize = 32 << 10;

  bool stopped_{};
  udp::socket socket_;
  udp::endpoint sender_ep_;
  std::vector<char> readbuff_;
  boost::asio::deadline_timer timer_;
  std::unordered_map<kcp_conv_t, KcpSessionPtr> sessions_;
  ConnectionFunction connection_fn_{};
  MessageFunction message_fn_{};

  void do_read(void);
  void do_timer(void);
  kcp_conv_t gen_conv(void) const;
public:
  KcpServer(boost::asio::io_service& io_service, std::uint16_t port);
  ~KcpServer(void);
  void write(const std::string& buf, const udp::endpoint& ep);
  void write(const char* buf, std::size_t len, const udp::endpoint& ep);

  void bind_connection_functor(const ConnectionFunction& fn) {
    connection_fn_ = fn;
  }

  void bind_connection_functor(ConnectionFunction&& fn) {
    connection_fn_ = std::move(fn);
  }

  void bind_message_functor(const MessageFunction& fn) {
    message_fn_ = fn;
  }

  void bind_message_functor(MessageFunction&& fn) {
    message_fn_ = std::move(fn);
  }
};

}
