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
#include "KcpTypes.h"

namespace KcpNet {

using namespace ::boost::asio::ip;
class KcpSessionManager;

class KcpSession : private boost::noncopyable {
  std::weak_ptr<KcpSessionManager> session_mgr_;
  kcp_conv_t conv_{};
  ikcpcb* kcp_{};
  udp::endpoint sender_ep_;

  void write_package_back(const std::string& package) {
    write_buffer(package);
  }

  void init_kcp(kcp_conv_t conv);
  void write_package(const char* buf, std::size_t len);
  static int output_callback(
      const char* buf, int len, ikcpcb* kcp, void* user);
public:
  using KcpSessionPtr = std::shared_ptr<KcpSession>;

  explicit KcpSession(const std::weak_ptr<KcpSessionManager>& session_mgr);
  ~KcpSession(void);

  void set_sender_endpoint(const udp::endpoint& sender_ep) {
    sender_ep_ = sender_ep;
  }

  static KcpSession::KcpSessionPtr create(
      const std::weak_ptr<KcpSessionManager>& session_mgr,
      kcp_conv_t conv, const udp::endpoint& sender_ep);
  void update(std::uint32_t clock);
  void read_buffer(
      const char* buf, std::size_t len, const udp::endpoint& sender_ep);
  void write_buffer(const std::string& buf);
};

}
