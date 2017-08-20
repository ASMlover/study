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
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include "Callbacks.h"

namespace KcpNet {

using boost::asio::ip::udp;

class KcpClient : private boost::noncopyable {
  static constexpr std::size_t kBufferSize = 32 << 10;
  static constexpr std::uint64_t kConnectTimeout = 5000;

  bool stopped_{};
  bool connecting_{};
  bool connected_{};
  std::uint64_t connect_begtime_{};
  udp::socket socket_;
  boost::asio::deadline_timer timer_;
  std::vector<char> readbuff_;
  KcpSessionPtr session_{};
  ConnectionFunction connection_fn_{};
  ConnectFailFunction connectfail_fn_{};
  MessageFunction message_fn_{};

  void do_write_connection(void);
  void do_read_connection(void);
  void do_read(void);
  void do_timer(void);
public:
  KcpClient(boost::asio::io_service& io_service, std::uint16_t port);
  ~KcpClient(void);
  void connect(const std::string& remote_ip, std::uint16_t remote_port);
  void write(const std::string& buf);
  void write(const char* buf, std::size_t len);

  void bind_connection_functor(const ConnectionFunction& fn) {
    connection_fn_ = fn;
  }

  void bind_connection_functor(ConnectionFunction&& fn) {
    connection_fn_ = std::move(fn);
  }

  void bind_connectfail_functor(const ConnectFailFunction& fn) {
    connectfail_fn_ = fn;
  }

  void bind_connectfail_functor(ConnectFailFunction&& fn) {
    connectfail_fn_ = std::move(fn);
  }

  void bind_message_functor(const MessageFunction& fn) {
    message_fn_ = fn;
  }

  void bind_message_functor(MessageFunction&& fn) {
    message_fn_ = std::move(fn);
  }
};

}
