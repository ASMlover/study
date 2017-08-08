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

#include "KcpTypes.h"
#include "KcpSessionContainer.h"

namespace KcpNet {

class KcpSessionManager
  : public std::enable_shared_from_this<KcpSessionManager>
  , private boost::noncopyable {
  static constexpr std::size_t kMaxPacket = 1080;

  bool stopped_{};
  char data_[1024 * 32]{};
  SMessageFunctor message_fn_{};
  udp::socket socket_;
  udp::endpoint sender_ep_;
  boost::asio::deadline_timer timer_;
  KcpSessionContainer container_;

  void do_async_receive(void);
  void do_timer(void);
  void handle_timer(void);
  void handle_connect_packet(void);
  void handle_packet(std::size_t n);
public:
  KcpSessionManager(boost::asio::io_service& io_service,
      const std::string& address, std::uint16_t port);

  void bind_meesage_functor(const SMessageFunctor& fn) {
    message_fn_ = fn;
  }

  void bind_meesage_functor(SMessageFunctor&& fn) {
    message_fn_ = std::move(fn);
  }

  void stop_all(void);
  void call_message_functor(
      kcp_conv_t conv, SMessageType type, const MessageBuffer& buf);
  void write_udp_buffer(const std::string& buf, const udp::endpoint& ep);
};

}
