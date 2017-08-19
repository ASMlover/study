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

#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include "Callbacks.h"
#include "KcpTypes.h"

namespace KcpNet {

using boost::asio::ip::udp;

class KcpSession
  : private boost::noncopyable
  , public std::enable_shared_from_this<KcpSession> {
  kcp_conv_t conv_{};
  ikcpcb* kcp_{};
  udp::endpoint sender_ep_;
  WriteFunction write_fn_{};
  MessageFunction message_fn_{};

  void init_kcp(void);
  static int output_handler(const char* buf, int len, ikcpcb* kcp, void* user);
  void write_impl(const char* buf, std::size_t len);
public:
  KcpSession(kcp_conv_t conv);
  KcpSession(kcp_conv_t conv, const udp::endpoint& sender_ep);
  ~KcpSession(void);

  void update(std::uint32_t clock);
  void input_handler(const char* buf, std::size_t len);
  void input_handler(const char* buf, std::size_t len,
      const udp::endpoint& sender_ep);
  void write_buffer(const std::string& buf);

  kcp_conv_t get_conv(void) const {
    return conv_;
  }

  const udp::endpoint& get_endpoint(void) const {
    return sender_ep_;
  }

  void set_sender_endpoint(const udp::endpoint& sender_ep) {
    sender_ep_ = sender_ep;
  }

  void set_sender_endpoint(udp::endpoint&& sender_ep) {
    sender_ep_ = std::move(sender_ep);
  }

  void bind_write_functor(const WriteFunction& fn) {
    write_fn_ = fn;
  }

  void bind_write_functor(WriteFunction&& fn) {
    write_fn_ = std::move(fn);
  }

  void bind_message_functor(const MessageFunction& fn) {
    message_fn_ = fn;
  }

  void bind_message_functor(MessageFunction&& fn) {
    message_fn_ = std::move(fn);
  }
};

}
